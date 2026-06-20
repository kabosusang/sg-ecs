#pragma once
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

namespace ecs {

class blob_array {
public:
	using drop_fn = void (*)(void* item);
	using move_fn = void (*)(void* dst, void* src);

	blob_array() = default;

	blob_array(size_t item_size, size_t item_align, drop_fn drop, move_fn move) noexcept
			: item_size_(item_size),
			  item_align_(item_align),
			  drop_(drop),
			  move_(move) {}

	blob_array(blob_array&& other) noexcept
			: data_(other.data_),
			  size_(other.size_),
			  capacity_(other.capacity_),
			  item_size_(other.item_size_),
			  item_align_(other.item_align_),
			  drop_(other.drop_),
			  move_(other.move_) {
		other.data_ = nullptr;
		other.size_ = other.capacity_ = 0;
	}

	blob_array& operator=(blob_array&& other) noexcept {
		if (this != &other) {
			clear();
			::operator delete(data_);
			data_ = other.data_;
			size_ = other.size_;
			capacity_ = other.capacity_;
			item_size_ = other.item_size_;
			item_align_ = other.item_align_;
			drop_ = other.drop_;
			move_ = other.move_;
			other.data_ = nullptr;
			other.size_ = other.capacity_ = 0;
		}
		return *this;
	}

	blob_array(const blob_array&) = delete;
	blob_array& operator=(const blob_array&) = delete;

	~blob_array() {
		clear();
		::operator delete(data_);
	}

	size_t size() const noexcept { return size_; }
	size_t capacity() const noexcept { return capacity_; }
	bool empty() const noexcept { return size_ == 0; }
	size_t item_size() const noexcept { return item_size_; }

	uint8_t* data() noexcept { return data_; }
	const uint8_t* data() const noexcept { return data_; }
	uint8_t* at(size_t index) noexcept { return data_ + index * item_size_; }
	const uint8_t* at(size_t index) const noexcept { return data_ + index * item_size_; }

	void reserve(size_t new_capacity) {
		if (new_capacity <= capacity_) {
			return;
		}
		size_t grow = (capacity_ == 0) ? 4 : capacity_ * 2;
		if (new_capacity > grow) {
			grow = new_capacity;
		}

		void* new_data = ::operator new(grow * item_size_,
				std::align_val_t{ item_align_ });
		uint8_t* new_data_ptr = static_cast<uint8_t*>(new_data);

		for (size_t i = 0; i < size_; ++i) {
			move_(new_data_ptr + i * item_size_, data_ + i * item_size_);
		}
		for (size_t i = 0; i < size_; ++i) {
			drop_(data_ + i * item_size_);
		}

		::operator delete(data_);
		data_ = new_data_ptr;
		capacity_ = grow;
	}

	void push_back(const void* src) {
		if (size_ == capacity_) {
			reserve(size_ + 1);
		}
		move_(at(size_), const_cast<void*>(src));
		++size_;
	}

	void swap_remove(size_t index) {
		size_t last = size_ - 1;
		if (index != last) {
			drop_(at(index));
			move_(at(index), at(last));
		}
		drop_(at(last));
		--size_;
	}

	void clear() {
		for (size_t i = 0; i < size_; ++i) {
			drop_(at(i));
		}
		size_ = 0;
	}

	void set(size_t index, const void* src) {
		drop_(at(index));
		move_(at(index), const_cast<void*>(src));
	}

	void pop_back() {
		swap_remove(size_ - 1);
	}

private:
	uint8_t* data_ = nullptr;
	size_t size_ = 0;
	size_t capacity_ = 0;
	size_t item_size_ = 0;
	size_t item_align_ = alignof(std::max_align_t);
	drop_fn drop_ = nullptr;
	move_fn move_ = nullptr;
};

template <typename T>
struct blob_array_accessor {
	static void drop(void* item) {
		static_cast<T*>(item)->~T();
	}
	static void move_construct(void* dst, void* src) {
		::new (dst) T(std::move(*static_cast<T*>(src)));
	}
	static blob_array make_blob_array() {
		return blob_array(sizeof(T), alignof(T), drop, move_construct);
	}
	static T& get(blob_array& ba, size_t index) noexcept {
		return *reinterpret_cast<T*>(ba.at(index));
	}
	static const T& get(const blob_array& ba, size_t index) noexcept {
		return *reinterpret_cast<const T*>(ba.at(index));
	}
};

} // namespace ecs
