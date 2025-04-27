//
// Created by sai on 20/4/2025.
//

#ifndef DEQUE_H
#define DEQUE_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>

template <typename T, typename Allocator = std::allocator<T>>
class deque {
 private:
  using allocator_type = Allocator;
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer =
      typename std::allocator_traits<Allocator>::const_pointer;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using map_allocator =
      typename std::allocator_traits<Allocator>::template rebind_alloc<pointer>;

  static const size_type block_size = 512 / sizeof(T) > 1 ? 512 / sizeof(T) : 1;

  template <bool IsConst>
  class deque_iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using pointer = typename std::conditional<IsConst, const T*, T*>::type;
    using reference = typename std::conditional<IsConst, const T&, T&>::type;

    deque_iterator()
        : current(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

    deque_iterator(pointer cur, pointer f, pointer l, pointer* n)
        : current(cur), first(f), last(l), node(n) {}

    reference operator*() const { return *current; }
    pointer operator->() const { return current; }

    reference operator[](difference_type n) const { return *(*this + n); }
    deque_iterator& operator++() {
      ++current;
      if (current == last) {
        set_node(node + 1);
        current = first;
      }
      return *this;
    }

    deque_iterator operator++(int) {
      deque_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    deque_iterator& operator--() {
      if (current == first) {
        set_node(node - 1);
        current = last;
      }
      --current;
      return *this;
    }

    deque_iterator operator--(int) {
      deque_iterator tmp = *this;
      --*this;
      return tmp;
    }

    deque_iterator& operator+=(difference_type n) {
      difference_type offset = n + (current - first);
      if (offset >= 0 && offset < difference_type(block_size)) {
        current += n;
      } else {
        difference_type node_offset =
            offset > 0 ? offset / difference_type(block_size)
                       : -difference_type((-offset - 1) / block_size) - 1;
        set_node(node_offset);
        current = first + (offset - node_offset * block_size);
      }
      return *this;
    }

    deque_iterator operator+(difference_type n) const {
      deque_iterator tmp = *this;
      return tmp += n;
    }

    friend deque_iterator operator+(difference_type n,
                                    const deque_iterator& it) {
      return it + n;
    }

    deque_iterator& operator-=(difference_type n) { return *this += (-n); }
    deque_iterator operator-(const deque_iterator& other) const {
      return block_size * (node - other.node) + (current - first) -
             (other.current - other.first);
    }
    bool operator==(const deque_iterator& other) const {
      return current == other.current;
    }
    bool operator!=(const deque_iterator& other) const {
      return !(*this == other);
    }
    bool operator<(const deque_iterator& other) const {
      return (node == other.node) ? (current < other.current)
                                  : (node < other.node);
    }

    pointer* get_node() const { return node; }

   private:
    void set_node(pointer* new_node) {
      node = new_node;
      first = *new_node;
      last = first + block_size;
    }

    pointer current;
    pointer first;
    pointer last;
    pointer* node;
  };

 public:
  using iterator = deque_iterator<false>;
  using const_iterator = deque_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  deque() : map(nullptr), map_size(0), start(), finish() {}
  explicit deque(size_type count, const T& value = T(),
                 const Allocator& allow = Allocator());
  explicit deque(const Allocator& alloc);
  deque(const deque& other);
  deque(deque&& other) noexcept;
  ~deque();

  deque& operator=(const deque& other);
  deque& operator=(deque&& other) noexcept;

  reference at(size_type pos);
  const_reference at(size_type pos) const;
  reference operator[](size_type pos);
  const_reference operator[](size_type pos) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;
  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  reverse_iterator renc() const noexcept;
  const_reverse_iterator crend() const noexcept;

  bool empty() const noexcept;
  size_type size() const noexcept;
  size_type max_size() const noexcept;
  void shrink_to_fit();

  void clear() noexcept;
  iterator insert(const_iterator pos, const T& value);
  iterator insert(const_iterator pos, T&& value);
  iterator erase(const_iterator pos);
  void push_back(const T& value);
  void push_back(T&& value);
  void push_front(const T& value);
  void push_front(T&& value);
  void pop_back();
  void pop_front();
  void resize(size_type count);
  void resize(size_type count, const T& value);
  void swap(deque& other) noexcept;

 private:
  void create_map_add_nodes(size_type num_elements);
  void reallocate_map(size_type nodes_to_add, bool add_at_front);
  pointer allocate_node();
  void deallocate_node(pointer p) noexcept;
  void destroy_nodes(pointer* nstart, pointer* nfinish) noexcept;
  void fill_initialize(size_type n, const T& value);

  pointer* map;
  size_type map_size;
  iterator start;
  iterator finish;
  Allocator allocator;
  map_allocator map_alloc;
};

template <typename T, typename Alloc>
bool operator==(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <typename T, typename Alloc>
bool operator!=(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <typename T, typename Alloc>
bool operator<(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <typename T, typename Alloc>
bool operator>(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <typename T, typename Alloc>
bool operator>=(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <typename T, typename Alloc>
void swap(deque<T, Alloc>& lhs, deque<T, Alloc>& rhs) noexcept;

#endif  // DEQUE_H
