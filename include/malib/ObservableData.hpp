#pragma once

#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>

namespace malib {

template <std::copyable T, std::size_t MaxSubscribers>
struct ObservableData {
  using NotificationCallback = std::function<void(ObservableData&)>;

  /**
   * @brief Updates the stored data with new data and notifies observers
   * @tparam U Type of data being updated (supports both lvalue and rvalue)
   * @param data New data to store
   * 
   * This method is thread-safe as it uses mutex locking.
   * After updating the data, it automatically notifies all registered observers.
   * Note that the call to `update` is not preferred in an ISR context.
   */
  template <typename U>
  void update(U&& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_ = std::forward<U>(data);
    notify();
  }

  const T& get() const { return data_; }

  /**
   * @brief Subscribes a callback to be called when the data is updated
   * @param callback Callback function to be called
   * 
   * This method is thread-safe as it uses mutex locking.
   * The callback function should have the following signature:
   * `void callback(ObservableData&)`
   * 
   * The callback function will be called with the updated ObservableData object.
   * Note that the call to `subscribe` is not preferred in an ISR context.
   */
  void subscribe(NotificationCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (subscriber_idx_ >= MaxSubscribers) {
      return;
    }
    subscribers_[subscriber_idx_] = callback;
    subscriber_idx_++;
  }

 private:
  void notify() {
    for (const auto& subscriber : subscribers_) {
      if (subscriber.has_value()) {
        subscriber.value()(*this);
      }
    }
  }

 private:
  T data_{};
  std::array<std::optional<NotificationCallback>, MaxSubscribers>
      subscribers_{};
  std::mutex mutex_{};
  std::size_t subscriber_idx_{0};
};

};  // namespace malib