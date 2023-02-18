#ifndef CYCLEFIFO_H
#define CYCLEFIFO_H

#include <iostream>
#include <memory>
using namespace std;

//---------------node------------------------------
template<typename T>
struct Node
{
    T elem;
    Node<T> *next;

public:
    Node(const T t, Node<T> *next) : elem(t), next(next) { }
    ~Node() = default;
};

//-------------cycle_fifo----------------------------------------------
template<typename T, typename A = std::allocator<Node<T>>>
class CycleFIFO
{
    int m_buff_size = 0;
    int m_count = 0;

    A allocator;
    Node<T> *head, *tail;

    using difference_type = ptrdiff_t;

public:
    CycleFIFO(int b_size) : m_buff_size(b_size), head(nullptr), tail(nullptr) { }

    ~CycleFIFO()
    {
        clear();
    }

    //---------------------push----------------------------
    void push(const T elem)
    {
        Node<T> *node = allocator.allocate(1);
        allocator.construct(node, elem, nullptr);

        if (head == nullptr) {
            head = tail = node;
            m_count++;
        } else if (m_count < m_buff_size) {
            node->next = head;
            tail->next = node;
            tail = node;
            m_count++;
        } else {
            Node<T> *head_temp = head->next;
            node->next = head->next;
            tail->next = node;
            tail = node;
            allocator.destroy(head);
            allocator.deallocate(head, 1);
            head = head_temp;
        }
    }
    //--------------------pop--------------------------------
    void pop()
    {
        if (head != nullptr) {
            m_count--;

            if (m_count == 0) {
                allocator.destroy(head);
                allocator.deallocate(head, 1);
                head = nullptr;
                tail = nullptr;
                return;
            }

            if (m_count == 1) {
                allocator.destroy(head);
                allocator.deallocate(head, 1);
                head = tail;
                return;
            }

            Node<T> *head_temp = head->next;
            tail->next = head->next;
            head->next = head_temp->next;
            head->elem = head_temp->elem;
            allocator.destroy(head_temp);
            allocator.deallocate(head_temp, 1);
        }
    }
    //------------------count---------------------------------
    int count() const
    {
        return m_count;
    }
    //----------------clear----------------------------------
    void clear()
    {
        while (head) {
            pop();
        }
    }
    //-----------------operator[]-----------------------------
    T &operator[](difference_type n)
    {
        if (head) {
            auto tmp = head;
            for (difference_type i = 0; i != n; ++i) {
                tmp = tmp->next;
            }
            return tmp->elem;
        }
        throw bad_alloc();
    }
};

#endif // CYCLEFIFO_H
