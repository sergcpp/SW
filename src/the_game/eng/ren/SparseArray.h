#ifndef SPARSE_ARRAY_H
#define SPARSE_ARRAY_H

#include <cassert>
#include <iterator>
#include <vector>

template <class T>
class SparseArray {
protected:
	struct Item {
		unsigned	used		: 1;
		unsigned	next_free	: 31;
		T			val;

		Item() : used(0), next_free(0) {}
	};

	std::vector<Item> array_;
	size_t first_free_;
	size_t size_;
public:
	SparseArray() : SparseArray(8) {}
	SparseArray(size_t size) : first_free_(0), size_(0) {
		Resize(size);
	}
	size_t Size() const {
		return size_;
	}
	size_t Capacity() const {
		return array_.size();
	}
	void Clear() {
		array_.clear();
		Resize(8);
	}
	void Resize(size_t new_size) {
		size_t prev_size = array_.size();
		array_.resize(new_size);
		if (new_size > prev_size) {
			for (size_t i = prev_size; i < new_size; i++) {
				array_[i].next_free = (unsigned)(i + 1);
			}
		}
	}
	template<class... Args>
	size_t Add(Args &&... args) {
		if (size_ >= array_.size() - 1) {
			Resize(array_.empty() ? 8 : array_.size() * 2);
		}
		size_t index = first_free_;
		Item &i = array_[index];
		i.used = 1;
		i.val = T(args...);
		first_free_ = i.next_free;
		size_++;
		return index;
	}
	void Remove(size_t i) {
		Item &it = array_[i];
		it.used = 0;
		it.val = T();
		it.next_free = (unsigned)first_free_;
		first_free_ = i;
		size_--;
	}
	T *Get(size_t i) {
		return &array_[i].val;
	}
	
	size_t NextIndex(size_t i) {
		while (++i < array_.size()) {
			if (array_[i].used) {
				break;
			}
		}
		return i;
	}
	T *Next(size_t i) {
		i = NextIndex(i);
		return i < array_.size() ? &array_[i].val : nullptr;
	}

	class SparseArrayIterator : public std::iterator<std::bidirectional_iterator_tag, T> {
		friend class SparseArray<T>;

		SparseArray<T> *container_;
		size_t index_;

		SparseArrayIterator(SparseArray<T> *c, size_t index) : container_(c), index_(index) {}
	public:
		T &operator*() {
			return *container_->Get(index_);
		}
		T *operator->() {
			return container_->Get(index_);
		}
		SparseArrayIterator &operator++() {
			index_ = container_->NextIndex(index_);
			return *this;
		}
		SparseArrayIterator operator++(int) { 
			SparseArrayIterator tmp(*this); 
			++(*this); 
			return tmp;
		}

		size_t index() const {
			return index_;
		}

		bool operator< (const SparseArrayIterator &rhs) { return (index_ - rhs.index_) < 0; }
		bool operator<=(const SparseArrayIterator &rhs) { return (index_ - rhs.index_) <= 0; }
		bool operator> (const SparseArrayIterator &rhs) { return (index_ - rhs.index_) > 0; }
		bool operator>=(const SparseArrayIterator &rhs) { return (index_ - rhs.index_) >= 0; }
		bool operator==(const SparseArrayIterator &rhs) { return  index_ == rhs.index_; }
		bool operator!=(const SparseArrayIterator &rhs) { return  index_ != rhs.index_; }
	};

	typedef SparseArrayIterator iterator;

	iterator begin() {
		size_t i = 0;
		while (i < array_.size()) {
			if (array_[i].used) {
				return iterator(this, i);
			}
			++i;
		}
		return end();
	}

	iterator end() {
		return iterator(this, array_.size());
	}

	iterator it_at(size_t index) {
		return iterator(this, index);
	}
};

#endif // SPARSE_ARRAY_H
