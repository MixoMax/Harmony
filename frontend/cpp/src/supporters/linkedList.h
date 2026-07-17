#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <functional>

template <typename T>
class LinkedListNode
{
public:
    T data;
    LinkedListNode* next;
    LinkedListNode* prev;
    LinkedListNode(const T& data) : data(data), next(nullptr), prev(nullptr) {}
    LinkedListNode(const T& data, LinkedListNode* next, LinkedListNode* prev) : data(data), next(next), prev(prev) {}

};

template <typename T>
class LinkedList
{
private:
    LinkedListNode<T>* head;
    LinkedListNode<T>* tail;
    size_t listSize;
public:
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}
    ~LinkedList()
    {
        LinkedListNode<T>* current = head;
        while (current)
        {
            LinkedListNode<T>* next = current->next;
            delete current;
            current = next;
        }
    }
    void append(const T& value)
    {
        LinkedListNode<T>* newNode = new LinkedListNode<T>{value, nullptr, tail};
        if (!head)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }
    void prepend(const T& value)
    {
        LinkedListNode<T>* newNode = new LinkedListNode<T>{value, head, nullptr};
        head = newNode;
        if (!tail)
        {
            tail = newNode;
        }
        listSize++;
    }
    size_t size() const
    {
        return listSize;
    }
    LinkedListNode<T>* getHead() const
    {
        return head;
    }
    LinkedListNode<T>* getTail() const{
        return tail;
    }

    void clear(std::function<bool(const T&)> shouldDelete)
    {
        LinkedListNode<T>* current = head;
        while (current)
        {
            if(shouldDelete(current->data)){
                delete current->data;
            }

            LinkedListNode<T>* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
        tail = nullptr;
        listSize = 0;
    }
    void remove(LinkedListNode<T>* node, std::function<bool(const T&)> shouldDelete)
    {
        if (!node) return;

        if (shouldDelete(node->data)) {
            delete node->data;
        }

        if (node->prev)
        {
            node->prev->next = node->next;
        }
        else
        {
            head = node->next;
        }

        if (node->next)
        {
            node->next->prev = node->prev;
        }
        else
        {
            tail = node->prev;
        }

        delete node;
        listSize--;
    }
};

#endif // !LINKEDLIST_H