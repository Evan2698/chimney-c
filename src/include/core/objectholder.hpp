#ifndef OBJECT_HOLDER_H_293847484837_394858
#define OBJECT_HOLDER_H_293847484837_394858

template <typename T>
class ObjectHolder
{
private:
    T *m_p;

public:
    ObjectHolder() : m_p(nullptr)
    {
    }
    explicit ObjectHolder(T *t) : m_p(t)
    {
    }

    ObjectHolder(ObjectHolder &other) = delete;
    ObjectHolder(ObjectHolder &&other) = delete;

    T *operator->()
    {
        return m_p;
    }

    T *get()
    {
        return m_p;
    }

    T *detach()
    {
        T *tmp = m_p;
        m_p = nullptr;
        return tmp;
    }

    void attach(T *t)
    {
        m_p = t;
    }

    ~ObjectHolder()
    {
        if (m_p != nullptr)
        {
            delete m_p;
            m_p = nullptr;
        }
    }
};

#endif