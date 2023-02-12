#include <iostream>
#include <map>
#include "cyclefifo.h"

using namespace std;

long double fact(int N)
{
    if (N < 0)
        return 0;
    if (N == 0)
        return 1;
    else
        return N * fact(N - 1);
}

template<size_t TypeSize>
size_t getSizeofBlock(size_t count)
{
    bool overflowPossible = TypeSize > 1;
    if (overflowPossible) {
        size_t maxPossible = static_cast<size_t>(-1) / TypeSize;
        if (count > maxPossible) {
            throw std::bad_alloc();
        }
        return count * TypeSize;
    }
    throw std::bad_alloc();
}

template<typename T, size_t size>
class my_alloc
{
public:
    using value_type = T;
    using const_reference = const T &;

    template<typename U>
    struct rebind
    {
        typedef my_alloc<U, size> other;
    };

    void *m_total_mem = nullptr;
    void *m_partial_mem = nullptr;
    size_t m_size;
    int m_allocations = 0;

    my_alloc() : m_size(size) { }

    ~my_alloc()
    {
        if (m_total_mem)
            free(m_total_mem);
    };

    T *allocate(size_t n)
    {
        if (n == 0)
            return nullptr;

        if (m_allocations == 10) {
            throw bad_alloc();
        }

        if (m_allocations == 0) {
            m_total_mem = malloc(getSizeofBlock<sizeof(T)>(m_size));
            m_partial_mem = m_total_mem;
            if (!m_partial_mem) {
                throw bad_alloc();
            }
        }
        size_t blockSize = getSizeofBlock<sizeof(T)>(m_size - m_allocations);
        if (std::align(alignof(T), sizeof(T), m_partial_mem, blockSize)) {
            T *result = reinterpret_cast<T *>(m_partial_mem);
            m_partial_mem = (T *)m_partial_mem + sizeof(T);
            return result;
        }
        throw bad_alloc();
    }
    void deallocate(T*, size_t)
    {
        if (m_allocations == 0) {
            free(m_total_mem);
            m_total_mem = nullptr;
        }
    }

    void construct(T *ptr, const_reference val)
    {
        ::new (reinterpret_cast<void *>(ptr)) T(val);
        m_allocations++;
    }

    template<class U, typename... Args>
    void construct(U *ptr, Args &&...args)
    {
        ::new (reinterpret_cast<void *>(ptr)) U(std::forward<Args>(args)...);
        m_allocations++;
    }

    void destroy(T *ptr)
    {
        ptr->~T();
        m_allocations--;
    }

    template<typename U>
    void destroy(U *ptr)
    {
        ptr->~U();
        m_allocations--;
    }

    inline bool operator==(const my_alloc<T, size> &other) const
    {
        return *this == other;
    }
    inline bool operator!=(const my_alloc<T, size> &other) const
    {
        return !(*this == other);
    }
};

int main()
{
    const int size = 10;

    map<int, int> normal_map;
    for (int i = 0; i < size; ++i) {
        normal_map[i] = fact(i);
    }

    map<int, int, less<int>, my_alloc<pair<int, int>, size>> alloc_map;
    for (int i = 0; i < size; ++i) {
        alloc_map[i] = fact(i);
    }

    cout << "---------------------normal map--------------------" << endl;
    for (auto pair : normal_map) {
        cout << get<0>(pair) << " " << get<1>(pair) << endl;
    }
    cout << "---------------------alloc map--------------------" << endl;
    for (auto pair : alloc_map) {
        cout << get<0>(pair) << " " << get<1>(pair) << endl;
    }

    CycleFIFO<int> normal_fifo(size);
    for (int i = 0; i < 10; ++i) {
        normal_fifo.push(i);
    }

    CycleFIFO<int, my_alloc<Node<int>, size>> alloc_fifo(size);
    for (int i = 0; i < size; ++i) {
        alloc_fifo.push(i);
    }

    cout << "---------------------normal fifo--------------------" << endl;
    for (int i = 0; i < size; ++i) {
        cout << i << " " << normal_fifo[i] << endl;
    }
    cout << "---------------------alloc fifo--------------------" << endl;
    for (int i = 0; i < size; ++i) {
        cout << i << " " << alloc_fifo[i] << endl;
    }

    return 0;
}
