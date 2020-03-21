#include<iostream>
#include<functional>
#include<vector>
#include<memory>
#include<list>


template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    using Element = std::pair<KeyType, ValueType>;
	static const size_t START_SIZE;
	static const size_t SIZE_MULTIPLICATOR;
	static const size_t SIZE_BORDER;
    struct DataMember {
        Element elem;
        typename std::list<Element*>::iterator iter;
    };
public:
    class iterator {
        friend class const_iterator;

        typename std::list<Element*>::iterator it;
    public:
        iterator() {}

        iterator(const typename std::list<Element *>::iterator it_) : it(it_) {}

        iterator &operator++() {
            ++it;
            return *this;
        }

        iterator operator++(int) {
            iterator temp(it);
            ++it;
            return temp;
        }

        std::pair<const KeyType, ValueType> &operator*() {
            return *(reinterpret_cast<std::pair<const KeyType, ValueType> *>(*it));
        }

        std::pair<const KeyType, ValueType> *operator->() {
            return reinterpret_cast<std::pair<const KeyType, ValueType> *> (*it);
        }

        bool operator==(iterator x) const {
            return it == x.it;
        }

        bool operator!=(iterator x) const {
            return it != x.it;
        }
    };

    class const_iterator {
        typename std::list<Element *>::const_iterator it;
    public:

        const_iterator() {}

        const_iterator(const typename std::list<Element *>::const_iterator it_) : it(it_) {}

        const_iterator(const typename std::list<Element *>::iterator it_) : it(it_) {}

        const_iterator(const iterator it_) : it(it_.it) {}

        const_iterator &operator++() {
            ++it;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp(it);
            ++it;
            return temp;
        }

        const std::pair<const KeyType, ValueType> &operator*() {
            return *(reinterpret_cast<std::pair<const KeyType, ValueType> *>(*it));
        }

        const std::pair<const KeyType, ValueType> *operator->() {
            return reinterpret_cast<std::pair<const KeyType, ValueType> *>(*it);
        }

        bool operator==(const_iterator x) const {
            return it == x.it;
        }

        bool operator!=(const_iterator x) const {
            return it != x.it;
        }
    };

private:
    //const size_t REHASH = 1e9+7;
    size_t sz;
    Hash hasher;
    std::vector<std::list<DataMember>> data;
    std::list<Element *> list;

    size_t hash(const KeyType &) const;

    void rebuild();

public:
    HashMap(const Hash &hasher_ = Hash());

    template<class ForwardIterator>
    HashMap(ForwardIterator, ForwardIterator, const Hash &hasher_ = Hash());

    HashMap(const std::initializer_list<Element> &, const Hash &hasher_ = Hash());

    HashMap(const HashMap &);

    size_t size() const;

    bool empty() const;

    void insert(const Element &);

    void erase(const KeyType &);

    ValueType &operator[](const KeyType &);

    const ValueType &at(const KeyType &) const;

    void clear();

    Hash hash_function() const;

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    iterator find(const KeyType &);

    const_iterator find(const KeyType &) const;
};

template<class KeyType, class ValueType, class Hash>
const size_t HashMap<KeyType, ValueType, Hash>::START_SIZE = 4;


template<class KeyType, class ValueType, class Hash>
const size_t HashMap<KeyType, ValueType, Hash>::SIZE_MULTIPLICATOR = 4;

template<class KeyType, class ValueType, class Hash>
const size_t HashMap<KeyType, ValueType, Hash>::SIZE_BORDER = 2;

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash &hasher_): hasher(hasher_) {
    sz = 0;
    data.resize(START_SIZE);
}

template<class KeyType, class ValueType, class Hash>
template<class ForwardIterator>
HashMap<KeyType, ValueType, Hash>::
HashMap(ForwardIterator first, ForwardIterator last, const Hash &hasher_): hasher(hasher_) {
    std::vector<Element > data_temp;
    while (first != last) {
        data_temp.emplace_back(*first);
        first++;
    }
    sz = 0;
    data.resize(std::max(data_temp.size(), START_SIZE) * SIZE_MULTIPLICATOR);
    for (const auto &elem: data_temp) {
        insert(elem);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::
HashMap(const std::initializer_list<Element> &init,
        const Hash &hasher_): hasher(hasher_) {
    std::vector<Element> data_temp;
    auto first = init.begin(), last = init.end();
    while (first != last) {
        data_temp.emplace_back(*first);
        first++;
    }
    sz = 0;
    data.resize(std::max(data_temp.size(), START_SIZE) * SIZE_MULTIPLICATOR);
    for (const auto &elem: data_temp) {
        insert(elem);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::
HashMap(const HashMap &init) {
    std::vector<Element> data_temp;
    auto first = init.begin(), last = init.end();
    while (first != last) {
        data_temp.emplace_back(*first);
        first++;
    }
    sz = 0;
    data.resize(std::max(data_temp.size(), START_SIZE) * SIZE_MULTIPLICATOR);
    for (const auto &elem: data_temp) {
        insert(elem);
    }
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::rebuild() {
    std::vector<Element> data_temp;
    for (const auto &container: data) {
        for (const auto &member: container)
            data_temp.emplace_back(member.elem);
    }
    size_t old_sz = data.size();
    list.clear();
    data.clear();
    data.resize(std::max(old_sz, START_SIZE) * SIZE_MULTIPLICATOR);
    sz = 0;
    for (const auto &elem: data_temp) {
        insert(elem);
    }
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::hash(const KeyType &key) const {
    return hasher(key) % data.size();
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return sz;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
    return sz == 0;
}


template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(const Element &elem) {
    if (sz + 1 >= data.size() / SIZE_BORDER)
        rebuild();
    size_t h = hash(elem.first);
    for (const auto &member : data[h]) {
        if (member.elem.first == elem.first) {
            return;
        }
    }
    DataMember member{elem, list.end()};
    data[h].emplace_back(member);
    list.emplace_back(&data[h].back().elem);
    data[h].back().iter = prev(list.end());
    sz++;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType &key) {
    size_t h = hash(key);

    for (auto it = data[h].begin(); it != data[h].end(); ++it) {
        if (it->elem.first == key) {
            list.erase(it->iter);
            data[h].erase(it);
            sz--;
            return;
        }
    }
}

template<class KeyType, class ValueType, class Hash>
ValueType &HashMap<KeyType, ValueType, Hash>::operator[](const KeyType &key) {
    if (sz + 1 >= data.size() / SIZE_BORDER)
        rebuild();
    size_t h = hash(key);

    for (auto &member : data[h]) {
        if (member.elem.first == key) {
            return member.elem.second;
        }
    }

    DataMember member{Element(key, ValueType()), list.end()};
    data[h].emplace_back(member);
    list.emplace_back(&data[h].back().elem);
    data[h].back().iter = prev(list.end());
    sz++;
    return data[h].back().elem.second;
}

template<class KeyType, class ValueType, class Hash>
const ValueType &HashMap<KeyType, ValueType, Hash>::at(const KeyType &key) const {
    size_t h = hash(key);

    for (const auto &member : data[h]) {
        if (member.elem.first == key) {
            return member.elem.second;
        }
    }

    throw std::out_of_range("");
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    list.clear();
    data.clear();
    sz = 0;
    data.resize(START_SIZE);
}

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return hasher;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
    return iterator(list.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const {
    return const_iterator(list.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
    return iterator(list.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const {
    return const_iterator(list.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType &key) {
    size_t h = hash(key);

    for (auto &member : data[h]) {
        if (member.elem.first == key) {
            return member.iter;
        }
    }

    return end();

}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType &key) const {
    size_t h = hash(key);

    for (const auto &member : data[h]) {
        if (member.elem.first == key) {
            return member.iter;
        }
    }

    return end();

}
