#ifndef HEAP_H
#define HEAP_H

#include <functional>
#include <vector>

template <typename T>
class Heap {
    std::vector<T> data;
    std::function<int(const T&, const T&)> comparator;
    size_t _parent_index(size_t index) {
        if(index == 0) {
            return 0;
        }

        return (index - 1) / 2;
    }
    size_t left_child_index(size_t index) {
        size_t left_child = (index * 2) + 1;
        return left_child < data.size()?left_child:index;
    }
    size_t right_child_index(size_t index) {
        size_t right_child = (index * 2) + 2;
        return right_child < data.size()?right_child:index;
    }
    public:
        Heap(std::function<int(const T&, const T&)> comparator) {
            this->comparator = comparator;
        }
        void push(T item) {
            data.push_back(item);

            size_t our_index = data.size() - 1;
            size_t parent_index = _parent_index(our_index);

            while(comparator(data[our_index], data[parent_index]) < 0) {
                T tmp = data[our_index];
                data[our_index] = data[parent_index];
                data[parent_index] = tmp;

                our_index = parent_index;
                parent_index = _parent_index(our_index);
            }
        }
        T pop() {
            T tmp = data[0];
            data[0] = data.end()[-1];
            data.end()[-1] = tmp;

            T ret = data.back();
            data.pop_back();

            size_t left_child_index = this->left_child_index(0);
            size_t right_child_index = this->right_child_index(0);
            size_t our_index = 0;

            while(comparator(data[our_index], data[left_child_index]) > 0
                || comparator(data[our_index], data[right_child_index]) > 0) {
                size_t smallest_child_index;
                if(comparator(data[left_child_index], data[right_child_index]) > 0) {
                    smallest_child_index = right_child_index;
                } else {
                    smallest_child_index = left_child_index;
                }

                T tmp = data[our_index];
                data[our_index] = data[smallest_child_index];
                data[smallest_child_index] = tmp;

                our_index = smallest_child_index;
                left_child_index = this->left_child_index(our_index);
                right_child_index = this->right_child_index(our_index);
            }
            
            return ret;
        }
        bool is_empty() {
            return data.size() == 0;
        }
        void clear() {
            data.clear();
        }
};

#endif