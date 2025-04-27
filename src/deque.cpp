//
// Created by sai on 20/4/2025.
//

#include "deque.h"

#include <algorithm>
#include <memory>

template <typename T, typename Allocator>
deque<T, Allocator>::deque(size_type count, const T& value,
                           const Allocator& alloc)
    : allocator(alloc), map_alloc(alloc) {
  create_map_add_nodes(count);
  fill_initialize(count, value);
}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const Allocator& alloc)
    : map(nullptr),
      map_size(0),
      start(),
      finish(),
      allocator(alloc),
      map_alloc(alloc) {}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const deque& other)
    : allocator(std::allocator_traits<Allocator>::
                    select_on_container_copy_construction(other.allocator)),
      map_alloc(std::allocator_traits<Allocator>::
                    select_on_container_copy_construction(other.allocator)) {
  create_map_add_nodes(other.size());
  try {
    std::uninitialized_copy(other.begin(), other.end(), start);
  } catch (...) {
    destroy_nodes(start.get_node(), finish.get_node() + 1);
    map_alloc.deallocate(map, map_size);
    throw;
  }
}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(deque&& other) noexcept
    : map(other.map),
      map_size(other.map_size),
      start(other.start),
      finish(other.finish),
      allocator(std::move(other.allocator)),
      map_alloc(std::move(other.map_alloc)) {
  other.map = nullptr;
  other.map_size = 0;
  other.start = iterator();
  other.finish = iterator();
}

template <typename T, typename Allocator>
deque<T, Allocator>::~deque() {
  if (map) {
    clear();
    destroy_nodes(start.get_node(), finish.get_node() + 1);
    map_alloc.deallocate(map, map_size);
  }
}

template <typename T, typename Allocator>
deque<T, Allocator>& deque<T, Allocator>::operator=(const deque& other) {
  if (this != &other) {
    deque tmp(other);
    swap(tmp);
  }
  return *this;
}

template <typename T, typename Allocator>
deque<T, Allocator>& deque<T, Allocator>::operator=(deque&& other) noexcept {
  if (this != &other) {
    clear();
    destroy_nodes(start.get_node(), finish.get_node() + 1);
    map_alloc.deallocate(map, map_size);

    map = other.map;
    map_size = other.map_size;
    start = other.start;
    finish = other.finish;
    allocator = std::move(other.allocator);
    map_alloc = std::move(other.map_alloc);

    other.map = nullptr;
    other.map_size = 0;
    other.start = iterator();
    other.finish = iterator();
  }
  return *this;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::reference deque<T, Allocator>::at(size_type pos) {
  if (pos >= size()) {
    throw std::out_of_range("deque::at");
  }
  return *(begin() + pos);
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_reference deque<T, Allocator>::at(
    size_type pos) const {
  if (pos >= size()) {
    throw std::out_of_range("deque::at");
  }
  return *(begin() + pos);
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::reference deque<T, Allocator>::operator[](
    size_type pos) {
  return start[pos];
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_reference deque<T, Allocator>::operator[](
    size_t pos) const {
  return *start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::reference deque<T, Allocator>::front() {
  return *start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_reference deque<T, Allocator>::front()
    const {
  return *start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::reference deque<T, Allocator>::back() {
  iterator tmp = finish;
  --tmp;
  return *tmp;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_reference deque<T, Allocator>::back()
    const {
  iterator tmp = finish;
  --tmp;
  return *tmp;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::iterator deque<T, Allocator>::begin() noexcept {
  return start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_iterator deque<T, Allocator>::begin()
    const noexcept {
  return start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::iterator deque<T, Allocator>::end() noexcept {
  return finish;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::const_iterator deque<T, Allocator>::cend()
    const noexcept {
  return finish;
}

template <typename T, typename Allocator>
bool deque<T, Allocator>::empty() const noexcept {
  return start == finish;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::size_type deque<T, Allocator>::size()
    const noexcept {
  return finish - start;
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::size_type deque<T, Allocator>::max_size()
    const noexcept {
  return std::allocator_traits<allocator_type>::max_size(allocator);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::clear() noexcept {
  for (pointer* node = start.get_node() + 1; node < finish.get_node(); ++node) {
    for (pointer p = *node; p != *node + block_size; ++p) {
      std::allocator_traits<Allocator>::destroy(allocator, p);
    }
  }
  if (start.get_node() != finish.get_node()) {
    for (pointer p = start.current; p != start.last; ++p) {
      std::allocator_traits<Allocator>::destroy(allocator, p);
    }
    for (pointer p = finish.first; p != finish.current; ++p) {
      std::allocator_traits<Allocator>::destroy(allocator, p);
    }
  } else {
    for (pointer p = start.current; p != finish.current; ++p) {
      std::allocator_traits<Allocator>::destroy(allocator, p);
    }
  }
  finish = start;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_back(const T& value) {
  if (finish.current != finish.last - 1) {
    std::allocator_traits<Allocator>::construct(allocator, finish.current,
                                                value);
    ++finish.current;
  } else {
    reallocate_map(1, false);
    *(finish.node + 1) = allocate_node();
    try {
      std::allocator_traits<Allocator>::construct(allocator, finish.current,
                                                  value);
      finish.set_node(finish.node + 1);
      finish.current = finish.first;
    } catch (...) {
      deallocate_node(*(finish.node + 1));
      throw;
    }
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_back(T&& value) {
  if (finish.current != finish.last - 1) {
    std::allocator_traits<Allocator>::construct(allocator, finish.current,
                                                std::move(value));
    ++finish.current;
  } else {
    reallocate_map(1, false);
    *(finish.node + 1) = allocate_node();
    try {
      std::allocator_traits<Allocator>::construct(allocator, finish.current,
                                                  std::move(value));
      finish.set_node(finish.node + 1);
      finish.current = finish.first;
    } catch (...) {
      deallocate_node(*(finish.node + 1));
    }
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_front(const T& value) {
  if (start.current != start.first) {
    std::allocator_traits<Allocator>::construct(allocator, start.current - 1,
                                                value);
    --start.current;
  } else {
    reallocate_map(1, true);
    *(start.node - 1) = allocate_node();
    try {
      start.set_node(start.node - 1);
      start.current = start.last - 1;
      std::allocator_traits<Allocator>::construct(allocator, start.current,
                                                  value);
    } catch (...) {
      start.set_node(start.node + 1);
      start.current = start.first;
      deallocate_node(*(start.node - 1));
      throw;
    }
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_front(T&& value) {
  if (start.current != start.first) {
    std::allocator_traits<Allocator>::construct(allocator, start.current - 1,
                                                std::move(value));
    --start.current;
  } else {
    reallocate_map(1, true);
    *(start.node - 1) = allocate_node();
    try {
      start.set_node(start.node - 1);
      start.current = start.last - 1;
      deallocate_node(*(start.node - 1));
    } catch (...) {
      start.set_node(start.node + 1);
      start.current = start.first;
      deallocate_node(*(start.node - 1));
      throw;
    }
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::pop_back() {
  if (finish.current != finish.first) {
    --finish.current;
    std::allocator_traits<Allocator>::destroy(allocator, finish.current);
  } else {
    deallocate_node(finish.first);
    finish.set_node(finish.node - 1);
    finish.current = finish.last - 1;
    std::allocator_traits<Allocator>::destroy(allocator, finish.current);
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::pop_front() {
  if (start.current != start.last - 1) {
    std::allocator_traits<Allocator>::destroy(allocator, start.current);
    ++start.current;
  } else {
    std::allocator_traits<Allocator>::destroy(allocator, start.current);
    deallocate_node(start.first);
    start.set_node(start.node + 1);
    start.current = start.first;
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::create_map_add_nodes(size_type num_elements) {
  size_type num_nodes = num_elements / block_size + 1;

  map_size = std::max(static_cast<size_type>(8), num_elements + 2);
  map = allocator.allocate(map_size);

  pointer* nstart = map + (map_size - num_nodes) / 2;
  pointer* nfinish = nstart + num_nodes;

  try {
    for (pointer* cur = nstart; cur < nfinish; ++cur) {
      *cur = allocate_node();
    }
  } catch (...) {
    for (pointer* cur = nstart; cur < nfinish; ++cur) {
      if (*cur) deallocate_node(*cur);
    }
    allocator.deallocate(map, max_size());
    throw;
  }

  start.set_node(nstart);
  finish.set_node(nfinish - 1);
  start.current = start.first;
  finish.current = finish.first + num_elements % block_size;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::reallocate_map(size_type nodes_to_add,
                                         bool add_at_front) {
  size_type old_num_nodes = finish.node - start.node + 1;
  size_type new_num_nodes = old_num_nodes + nodes_to_add;

  pointer* new_nstart;
  if (map_size > 2 * new_num_nodes) {
    new_nstart = map + (map_size - new_num_nodes) / 2 +
                 (add_at_front ? nodes_to_add : 0);
    if (new_nstart < start.node) {
      std::copy(start.node, finish.node + 1, new_nstart);
    } else {
      std::copy_backward(start.node, finish.node + 1,
                         new_nstart + old_num_nodes);
    }
  } else {
    size_type new_map_size = map_size + std::max(map_size, nodes_to_add) + 2;
    pointer* new_map = allocator.allocate(new_map_size);
    new_nstart = new_map + (new_map_size - new_num_nodes) / 2 +
                 (add_at_front ? nodes_to_add : 0);
    std::copy(start.node, finish.node + 1, new_nstart);
    allocator.deallocate(map, map_size);

    map = new_map;
    map_size = new_map_size;
  }

  start.set_node(new_nstart);
  finish.set_node(new_nstart + old_num_nodes - 1);
}

template <typename T, typename Allocator>
typename deque<T, Allocator>::pointer deque<T, Allocator>::allocate_node() {
  return std::allocator_traits<Allocator>::allocate(allocator, block_size);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::deallocate_node(pointer p) noexcept {
  std::allocator_traits<Allocator>::deallocate(allocator, p, block_size);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::destroy_nodes(pointer* nstart,
                                        pointer* nfinish) noexcept {
  for (pointer* node = nstart; node < nfinish; ++node) {
    deallocate_node(*node);
  }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::fill_initialize(size_type n, const T& value) {
  for (pointer* cur = start.node; cur < finish.node; ++cur) {
    std::uninitialized_fill(*cur, *cur + block_size, value);
  }
  std::uninitialized_fill(finish.first, finish.current, value);
}

template class deque<int>;
template class deque<double>;
template class deque<std::string>;