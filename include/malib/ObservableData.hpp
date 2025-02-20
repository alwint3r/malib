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

  template <typename U>
  void update(U&& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_ = std::forward<U>(data);
    notify();
  }

  const T& get() const { return data_; }

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