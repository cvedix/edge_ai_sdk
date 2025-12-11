// Implementation of cvedix_mqtt_json_receiver and cvedix_mqtt_client using mosquitto
// These are replacement implementations since the SDK doesn't provide them

#ifdef CVEDIX_WITH_MQTT

#include "cvedix/utils/mqtt_json_receiver/cvedix_mqtt_json_receiver.h"
#include "cvedix/utils/mqtt_client/cvedix_mqtt_client.h"
#include "third_party/nlohmann/json.hpp"
#include <mosquitto.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>

using json = nlohmann::json;

namespace cvedix_utils {

// Helper struct to store message callbacks
struct mqtt_client_callbacks {
    cvedix_mqtt_client::on_connect_callback on_connect_cb;
    cvedix_mqtt_client::on_disconnect_callback on_disconnect_cb;
    cvedix_mqtt_client::on_message_callback on_message_cb;
    cvedix_mqtt_client::on_publish_callback on_publish_cb;
    cvedix_mqtt_client* client_ptr;
};

// Static map to store callbacks
static std::mutex callback_map_mutex;
static std::map<struct mosquitto*, mqtt_client_callbacks> mosq_to_callbacks_map;

// Static callbacks for mosquitto
static void mqtt_on_connect(struct mosquitto* mosq, void* obj, int rc) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    auto it = mosq_to_callbacks_map.find(mosq);
    if (it != mosq_to_callbacks_map.end() && it->second.on_connect_cb) {
        it->second.on_connect_cb(rc == 0);
    }
}

static void mqtt_on_disconnect(struct mosquitto* mosq, void* obj, int rc) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    auto it = mosq_to_callbacks_map.find(mosq);
    if (it != mosq_to_callbacks_map.end() && it->second.on_disconnect_cb) {
        it->second.on_disconnect_cb();
    }
}

static void mqtt_on_message(struct mosquitto* mosq, void* obj, const struct mosquitto_message* message) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    auto it = mosq_to_callbacks_map.find(mosq);
    if (it != mosq_to_callbacks_map.end() && it->second.on_message_cb && message->payloadlen > 0) {
        std::string topic(message->topic);
        std::string payload(static_cast<const char*>(message->payload), message->payloadlen);
        it->second.on_message_cb(topic, payload);
    }
}

static void mqtt_on_publish(struct mosquitto* mosq, void* obj, int mid) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    auto it = mosq_to_callbacks_map.find(mosq);
    if (it != mosq_to_callbacks_map.end() && it->second.on_publish_cb) {
        it->second.on_publish_cb(mid);
    }
}

// Implementation of cvedix_mqtt_client
cvedix_mqtt_client::cvedix_mqtt_client(
    const std::string& broker_url,
    int port,
    const std::string& client_id,
    int keepalive)
    : broker_url_(broker_url), port_(port), 
      client_id_(client_id.empty() ? "cvedix_mqtt_client" : client_id),
      keepalive_(keepalive), connected_(false), connecting_(false),
      auto_reconnect_enabled_(false), reconnect_interval_ms_(5000),
      should_stop_reconnect_(false), mosq_(nullptr) {
    
    static std::once_flag lib_init_flag;
    std::call_once(lib_init_flag, []() {
        mosquitto_lib_init();
    });
    
    mosq_ = mosquitto_new(client_id_.c_str(), true, this);
    if (mosq_) {
        mosquitto_connect_callback_set(mosq_, mqtt_on_connect);
        mosquitto_disconnect_callback_set(mosq_, mqtt_on_disconnect);
        mosquitto_message_callback_set(mosq_, mqtt_on_message);
        mosquitto_publish_callback_set(mosq_, mqtt_on_publish);
        
        // Register in map
        std::lock_guard<std::mutex> lock(callback_map_mutex);
        mosq_to_callbacks_map[mosq_].client_ptr = this;
    }
}

cvedix_mqtt_client::~cvedix_mqtt_client() {
    should_stop_reconnect_ = true;
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
    
    if (mosq_) {
        std::lock_guard<std::mutex> lock(callback_map_mutex);
        mosq_to_callbacks_map.erase(mosq_);
        
        mosquitto_loop_stop(mosq_, false);
        mosquitto_disconnect(mosq_);
        mosquitto_destroy(mosq_);
    }
}

bool cvedix_mqtt_client::connect(const std::string& username, const std::string& password) {
    if (!mosq_) return false;
    
    username_ = username;
    password_ = password;
    
    if (!username.empty() && !password.empty()) {
        mosquitto_username_pw_set(mosq_, username.c_str(), password.c_str());
    }
    
    connecting_ = true;
    int rc = mosquitto_connect(mosq_, broker_url_.c_str(), port_, keepalive_);
    if (rc == MOSQ_ERR_SUCCESS) {
        connected_ = true;
        connecting_ = false;
        mosquitto_loop_start(mosq_);
        
        if (auto_reconnect_enabled_) {
            should_stop_reconnect_ = false;
            reconnect_thread_ = std::thread([this]() {
                while (!should_stop_reconnect_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_interval_ms_));
                    if (!connected_ && !should_stop_reconnect_) {
                        std::lock_guard<std::mutex> lock(publish_mutex_);
                        if (mosq_) {
                            int rc = mosquitto_connect(mosq_, broker_url_.c_str(), port_, keepalive_);
                            if (rc == MOSQ_ERR_SUCCESS) {
                                connected_ = true;
                            }
                        }
                    }
                }
            });
        }
        return true;
    } else {
        connecting_ = false;
        last_error_ = mosquitto_strerror(rc);
        return false;
    }
}

void cvedix_mqtt_client::disconnect() {
    should_stop_reconnect_ = true;
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
    
    if (mosq_) {
        mosquitto_loop_stop(mosq_, false);
        mosquitto_disconnect(mosq_);
        connected_ = false;
    }
}

bool cvedix_mqtt_client::is_connected() const {
    return connected_;
}

int cvedix_mqtt_client::publish(const std::string& topic, const std::string& payload, int qos, bool retain) {
    if (!mosq_ || !connected_) return -1;
    
    std::lock_guard<std::mutex> lock(publish_mutex_);
    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(), payload.length(), payload.c_str(), qos, retain);
    if (rc == MOSQ_ERR_SUCCESS && on_publish_cb_) {
        on_publish_cb_(rc);
    }
    return rc;
}

bool cvedix_mqtt_client::subscribe(const std::string& topic, int qos) {
    if (!mosq_ || !connected_) return false;
    int rc = mosquitto_subscribe(mosq_, nullptr, topic.c_str(), qos);
    return rc == MOSQ_ERR_SUCCESS;
}

std::string cvedix_mqtt_client::get_last_error() const {
    return last_error_;
}

void cvedix_mqtt_client::set_on_connect_callback(on_connect_callback cb) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    if (mosq_) {
        mosq_to_callbacks_map[mosq_].on_connect_cb = cb;
    }
    on_connect_cb_ = cb;
}

void cvedix_mqtt_client::set_on_disconnect_callback(on_disconnect_callback cb) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    if (mosq_) {
        mosq_to_callbacks_map[mosq_].on_disconnect_cb = cb;
    }
    on_disconnect_cb_ = cb;
}

void cvedix_mqtt_client::set_on_publish_callback(on_publish_callback cb) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    if (mosq_) {
        mosq_to_callbacks_map[mosq_].on_publish_cb = cb;
    }
    on_publish_cb_ = cb;
}

void cvedix_mqtt_client::set_on_message_callback(on_message_callback cb) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    if (mosq_) {
        mosq_to_callbacks_map[mosq_].on_message_cb = cb;
    }
    on_message_cb_ = cb;
}

void cvedix_mqtt_client::set_auto_reconnect(bool enable, int reconnect_interval_ms) {
    auto_reconnect_enabled_ = enable;
    reconnect_interval_ms_ = reconnect_interval_ms;
}

// Implementation of cvedix_mqtt_json_receiver
cvedix_mqtt_json_receiver::cvedix_mqtt_json_receiver(
    const std::string& broker_url,
    int port,
    const std::string& client_id)
    : mqtt_client_(std::make_unique<cvedix_mqtt_client>(broker_url, port, client_id.empty() ? "cvedix_json_receiver" : client_id)) {
    
    // Set message callback to handle messages
    mqtt_client_->set_on_message_callback([this](const std::string& topic, const std::string& payload) {
        handle_message(topic, payload);
    });
}

cvedix_mqtt_json_receiver::~cvedix_mqtt_json_receiver() = default;

bool cvedix_mqtt_json_receiver::connect(const std::string& username, const std::string& password) {
    return mqtt_client_->connect(username, password);
}

void cvedix_mqtt_json_receiver::disconnect() {
    mqtt_client_->disconnect();
}

bool cvedix_mqtt_json_receiver::subscribe(const std::string& topic, int qos) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    if (mqtt_client_->subscribe(topic, qos)) {
        subscribed_topics_.push_back(topic);
        return true;
    }
    return false;
}

bool cvedix_mqtt_json_receiver::subscribe_multiple(const std::vector<std::string>& topics, int qos) {
    bool all_success = true;
    std::lock_guard<std::mutex> lock(topics_mutex_);
    for (const auto& topic : topics) {
        if (mqtt_client_->subscribe(topic, qos)) {
            subscribed_topics_.push_back(topic);
        } else {
            all_success = false;
        }
    }
    return all_success;
}

bool cvedix_mqtt_json_receiver::unsubscribe(const std::string& topic) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    subscribed_topics_.erase(
        std::remove(subscribed_topics_.begin(), subscribed_topics_.end(), topic),
        subscribed_topics_.end()
    );
    return true;
}

void cvedix_mqtt_json_receiver::set_json_callback(json_callback callback) {
    json_cb_ = callback;
}

void cvedix_mqtt_json_receiver::set_raw_callback(raw_callback callback) {
    raw_cb_ = callback;
}

bool cvedix_mqtt_json_receiver::is_connected() const {
    return mqtt_client_->is_connected();
}

void cvedix_mqtt_json_receiver::set_auto_reconnect(bool enable, int reconnect_interval_ms) {
    mqtt_client_->set_auto_reconnect(enable, reconnect_interval_ms);
}

std::string cvedix_mqtt_json_receiver::get_last_error() const {
    return mqtt_client_->get_last_error();
}

bool cvedix_mqtt_json_receiver::is_valid_json(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        (void)j; // Suppress unused variable warning
        return true;
    } catch (const json::parse_error&) {
        return false;
    }
}

void cvedix_mqtt_json_receiver::handle_message(const std::string& topic, const std::string& payload) {
    // Call raw callback
    if (raw_cb_) {
        raw_cb_(topic, payload);
    }
    
    // Try to parse as JSON and call json callback
    if (json_cb_) {
        if (is_valid_json(payload)) {
            json_cb_(topic, payload);
        }
    }
}

void cvedix_mqtt_json_receiver::resubscribe_all() {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    for (const auto& topic : subscribed_topics_) {
        mqtt_client_->subscribe(topic, 1);
    }
}

} // namespace cvedix_utils

#endif // CVEDIX_WITH_MQTT
