#pragma once

#include <algorithm>
#include <forward_list>
#include <stdexcept>
#include <string>
#include <vector>

#define THROW(type, message) throw type(std::string(__FILE__) + " at line " + std::to_string(__LINE__) + ": " + message)
// i.hate.snake.case....
template <class TKey, class TValue, class THash = std::hash<TKey>>
class HashMap {
public:
    using TNode = std::pair<const TKey, TValue>;
    using TContainer = std::vector<std::forward_list<TNode>>;

    // Using default c++ container types
    // That way we can substitute HashMap into other template functions
    using key_type = TKey;
    using value_type = TValue;
    using mapped_type = TNode;

    // We start with size of 128 to prevent frequent resizings in the beginning
    static const size_t initialSize = 128;
    // Increase container when number of elements approaches size of container / maxLoadFactor
    // Decrease container when number of elements approaches size of container / maxLoadFactor^2
    static const size_t maxLoadFactor = 4;

    class iterator {
    public:
        using difference_type = long;
        using value_type = TNode;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        TContainer* mContainer;
        typename TContainer::iterator mContainerIterator;
        typename std::forward_list<TNode>::iterator mBucketIterator;

        iterator() = default;
        iterator& operator=(const iterator& other) = default;

        iterator& operator++();
        const iterator operator++(int);

        TNode& operator*();
        typename std::forward_list<TNode>::iterator operator->();

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
    };

    class const_iterator {
    public:
        using difference_type = long;
        using value_type = const TNode;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const TContainer* mContainer;
        typename TContainer::const_iterator mContainerIterator;
        typename std::forward_list<TNode>::const_iterator mBucketIterator;

        const_iterator() = default;
        const_iterator& operator=(const const_iterator& other) = default;

        const TNode& operator*() const;
        typename std::forward_list<TNode>::const_iterator operator->();

        const_iterator& operator++();
        const const_iterator operator++(int);

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    explicit HashMap(THash hash = THash{});
    template <typename IteratorType>
    HashMap(IteratorType begin, IteratorType end, THash hash = THash{});
    HashMap(const std::initializer_list<TNode>& list, THash hash = THash{});
    HashMap(const HashMap& other);
    HashMap& operator=(const HashMap& other);

    size_t size() const;
    bool empty() const;
    THash hash_function() const;

    void insert(TNode node);
    void erase(const TKey& key);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    iterator find(const TKey& key);
    const_iterator find(const TKey& key) const;

    TValue& operator[](const TKey& key);
    const TValue& at(const TKey& key) const;

    void clear();
    void resize(size_t newSize);

private:
    TContainer mContainer;
    THash mHasher;
    size_t mSize{};
    typename TContainer::iterator mBeginIterator;
};

template <class TKey, class TValue, class THash>
HashMap<TKey, TValue, THash>::HashMap(THash hash) : mHasher(hash) {
    clear();
}

template <class TKey, class TValue, class THash>
template <typename IteratorType>
HashMap<TKey, TValue, THash>::HashMap(IteratorType begin, IteratorType end, THash hash) : HashMap(hash) {
    resize(std::distance(begin, end));
    for (auto iter = begin; iter != end; ++iter) {
        insert(*iter);
    }
}

template <class TKey, class TValue, class THash>
HashMap<TKey, TValue, THash>::HashMap(const std::initializer_list<TNode>& list, THash hash) : HashMap(list.begin(), list.end(), hash) {
}

template <class TKey, class TValue, class THash>
HashMap<TKey, TValue, THash>::HashMap(const HashMap& other) : HashMap(other.begin(), other.end(), other.hash_function()) {
}

template <class TKey, class TValue, class THash>
HashMap<TKey, TValue, THash>& HashMap<TKey, TValue, THash>::operator=(const HashMap& other) {
    if (this == &other) {
        return *this;
    }
    clear();
    resize(other.size());
    mHasher = other.mHasher;
    for (const auto& i : other) {
        insert(i);
    }
    return *this;
}

template <class TKey, class TValue, class THash>
size_t HashMap<TKey, TValue, THash>::size() const {
    return mSize;
}

template <class TKey, class TValue, class THash>
bool HashMap<TKey, TValue, THash>::empty() const {
    return mSize == 0;
}

template <class TKey, class TValue, class THash>
THash HashMap<TKey, TValue, THash>::hash_function() const {
    return mHasher;
}

template <class TKey, class TValue, class THash>
void HashMap<TKey, TValue, THash>::insert(HashMap::TNode node) {
    if (find(node.first) != end()) {
        return;
    }

    size_t keyHash = mHasher(node.first) % mContainer.size();
    mContainer[keyHash].push_front(std::move(node));
    ++mSize;
    mBeginIterator = std::min(mBeginIterator, std::next(mContainer.begin(), keyHash));

    if (maxLoadFactor * size() >= mContainer.size()) {
        resize(mContainer.size() * maxLoadFactor);
    }
}

template <class TKey, class TValue, class THash>
void HashMap<TKey, TValue, THash>::erase(const TKey& key) {
    size_t keyHash = mHasher(key) % mContainer.size();
    for (const auto& i : mContainer[keyHash]) {
        if (i.first == key) {
            mContainer[keyHash].remove(i);
            --mSize;

            if (empty()) {
                clear();
            } else {
                if (mBeginIterator == std::next(mContainer.begin(), keyHash) && mContainer[keyHash].empty()) {
                    mBeginIterator = std::find_if(mBeginIterator, mContainer.end(), [](const auto& obj) {
                        return !obj.empty();
                    });
                }
                if (size() * maxLoadFactor <= mContainer.size() / maxLoadFactor) {
                    resize(mContainer.size() / maxLoadFactor);
                }
            }
            return;
        }
    }
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::iterator HashMap<TKey, TValue, THash>::begin() {
    return {
            .mContainer = &mContainer,
            .mContainerIterator = mBeginIterator,
            .mBucketIterator = mBeginIterator->begin()
    };
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::iterator HashMap<TKey, TValue, THash>::end() {
    return {
            .mContainer = &mContainer,
            .mContainerIterator = std::prev(mContainer.end()),
            .mBucketIterator = std::prev(mContainer.end())->end()
    };
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::iterator HashMap<TKey, TValue, THash>::find(const TKey& key) {
    size_t keyHash = mHasher(key) % mContainer.size();
    for (auto iter = mContainer[keyHash].begin(); iter != mContainer[keyHash].end(); ++iter) {
        if (iter->first == key) {
            return {
                    .mContainer = &mContainer,
                    .mContainerIterator = std::next(mContainer.begin(), keyHash),
                    .mBucketIterator = iter
            };
        }
    }
    return end();
}

template <class TKey, class TValue, class THash>
TValue& HashMap<TKey, TValue, THash>::operator[](const TKey& key) {
    auto iter = find(key);
    if (iter == end()) {
        insert({key, TValue{}});
        return find(key)->second;
    } else {
        return iter->second;
    }
}

template <class TKey, class TValue, class THash>
const TValue& HashMap<TKey, TValue, THash>::at(const TKey& key) const {
    auto iter = find(key);
    if (iter == end()) {
        THROW(std::out_of_range, "Invalid key: out of range");
    } else {
        return iter->second;
    }
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::const_iterator HashMap<TKey, TValue, THash>::begin() const {
    return {
            .mContainer = &mContainer,
            .mContainerIterator = mBeginIterator,
            .mBucketIterator = mBeginIterator->begin()
    };
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::const_iterator HashMap<TKey, TValue, THash>::end() const {
    return {
            .mContainer = &mContainer,
            .mContainerIterator = std::prev(mContainer.end()),
            .mBucketIterator = std::prev(mContainer.end())->end()
    };
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::const_iterator HashMap<TKey, TValue, THash>::find(const TKey& key) const {
    size_t keyHash = mHasher(key) % mContainer.size();
    for (auto iter = mContainer[keyHash].begin(); iter != mContainer[keyHash].end(); ++iter) {
        if (iter->first == key) {
            return {
                    .mContainer = &mContainer,
                    .mContainerIterator = std::next(mContainer.begin(), keyHash),
                    .mBucketIterator = iter
            };
        }
    }
    return end();
}

template <class TKey, class TValue, class THash>
void HashMap<TKey, TValue, THash>::clear() {
    mContainer.clear();
    mSize = 0;
    mContainer.resize(initialSize);
    mBeginIterator = std::prev(mContainer.end());
}

template <class TKey, class TValue, class THash>
void HashMap<TKey, TValue, THash>::resize(size_t newSize) {
    HashMap<TKey, TValue, THash> newContainer(mHasher);
    newContainer.mContainer.resize(newSize);
    newContainer.mBeginIterator = std::prev(newContainer.mContainer.end());

    for (const auto& i : *this) {
        newContainer.insert(i);
    }

    clear();
    mContainer = std::move(newContainer.mContainer);
    mSize = newContainer.mSize;
    mBeginIterator = newContainer.mBeginIterator;
}


template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::TNode& HashMap<TKey, TValue, THash>::iterator::operator*() {
    return *mBucketIterator;
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::iterator& HashMap<TKey, TValue, THash>::iterator::operator++() {
    if (std::next(mBucketIterator) == mContainerIterator->end() && std::next(mContainerIterator) != mContainer->end()) {
        ++mContainerIterator;
        while (std::next(mContainerIterator) != mContainer->end() && mContainerIterator->empty()) {
            ++mContainerIterator;
        }
        mBucketIterator = mContainerIterator->begin();
    } else {
        mBucketIterator = std::next(mBucketIterator);
    }
    return *this;
}

template <class TKey, class TValue, class THash>
const typename HashMap<TKey, TValue, THash>::iterator HashMap<TKey, TValue, THash>::iterator::operator++(int) {
    iterator it = *this;
    ++(*this);
    return it;
}

template <class TKey, class TValue, class THash>
bool HashMap<TKey, TValue, THash>::iterator::operator==(const HashMap::iterator& other) const {
    return mContainer == other.mContainer && mContainerIterator == other.mContainerIterator && mBucketIterator == other.mBucketIterator;
}

template <class TKey, class TValue, class THash>
bool HashMap<TKey, TValue, THash>::iterator::operator!=(const HashMap::iterator& other) const {
    return !(*this == other);
}

template <class TKey, class TValue, class THash>
typename std::forward_list<typename HashMap<TKey, TValue, THash>::TNode>::iterator
HashMap<TKey, TValue, THash>::iterator::operator->() {
    return mBucketIterator;
}

template <class TKey, class TValue, class THash>
const typename HashMap<TKey, TValue, THash>::TNode& HashMap<TKey, TValue, THash>::const_iterator::operator*() const {
    return *mBucketIterator;
}

template <class TKey, class TValue, class THash>
typename HashMap<TKey, TValue, THash>::const_iterator& HashMap<TKey, TValue, THash>::const_iterator::operator++() {
    if (std::next(mBucketIterator) == mContainerIterator->end() && std::next(mContainerIterator) != mContainer->end()) {
        ++mContainerIterator;
        while (std::next(mContainerIterator) != mContainer->end() && mContainerIterator->empty()) {
            ++mContainerIterator;
        }
        mBucketIterator = mContainerIterator->begin();
    } else {
        ++mBucketIterator;
    }
    return *this;
}

template <class TKey, class TValue, class THash>
const typename HashMap<TKey, TValue, THash>::const_iterator HashMap<TKey, TValue, THash>::const_iterator::operator++(int) {
    const_iterator it = *this;
    ++(*this);
    return it;
}

template <class TKey, class TValue, class THash>
bool HashMap<TKey, TValue, THash>::const_iterator::operator==(const HashMap::const_iterator& other) const {
    return mContainer == other.mContainer && mContainerIterator == other.mContainerIterator && mBucketIterator == other.mBucketIterator;
}

template <class TKey, class TValue, class THash>
bool HashMap<TKey, TValue, THash>::const_iterator::operator!=(const HashMap::const_iterator& other) const {
    return !(*this == other);
}

template <class TKey, class TValue, class THash>
typename std::forward_list<typename HashMap<TKey, TValue, THash>::TNode>::const_iterator HashMap<TKey, TValue, THash>::const_iterator::operator->() {
    return mBucketIterator;
}

#undef THROW