// Klopov Aleksei - st130153@student.spbu.ru

#ifndef SKIP_LIST
#define SKIP_LIST

#include <vector>
#include <optional>
#include <memory>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <utility>

/**
 * @class skip_list
 * @brief A skip list container that provides logarithmic time complexity for search operations.
 *
 * @tparam T The type of elements stored in the container.
 * @tparam Compare Comparison function object type (default: std::less<T>).
 * @tparam Allocator Allocator type (default: std::allocator<T>).
 */
template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
class skip_list
{
public:
    // Forward declaration
    class iterator;
    class const_iterator;

private:
    static constexpr int MAX_LEVEL = 32;  ///< Maximum levels for skip list nodes

    /// @struct skip_list_node
    /// @brief Internal node structure for the skip list
    struct skip_list_node
    {
        std::optional<T> value;          ///< Stored value (optional for sentinel nodes)
        int level;                       ///< Number of levels this node participates in
        std::vector<skip_list_node*> next; ///< Pointers to next nodes at each level
        skip_list_node* prev;             ///< Pointer to previous node at base level

        /// Constructor for sentinel nodes
        skip_list_node(int lvl) : value(std::nullopt), level(lvl), next(lvl, nullptr), prev(nullptr) {}

        /// Constructor for value nodes
        skip_list_node(const T& val, int lvl) : value(val), level(lvl), next(lvl, nullptr), prev(nullptr) {}

        /// Move constructor for value nodes
        skip_list_node(T&& val, int lvl) : value(std::move(val)), level(lvl), next(lvl, nullptr), prev(nullptr) {}
    };

    using node_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<skip_list_node>;
    using node_allocator_traits = std::allocator_traits<node_allocator_type>;

    Compare comp_;                       ///< Comparison function object
    node_allocator_type alloc_;          ///< Allocator for node memory management
    skip_list_node* head_;               ///< Head sentinel node
    skip_list_node* tail_;               ///< Tail sentinel node
    std::size_t size_;                   ///< Number of elements in container
    std::mt19937 gen_;                   ///< Random number generator
    std::uniform_real_distribution<> dis_; ///< Uniform distribution for random level generation

    /**
     * @brief Generates random level for new nodes
     * @return Random level between 1 and MAX_LEVEL
     */
    int random_level()
    {
        int level = 1;
        while (dis_(gen_) < 0.5 && level < MAX_LEVEL)
        {
            level++;
        }
        return level;
    }

    /**
     * @brief Initializes sentinel nodes
     */
    void initialize_sentinels()
    {
        head_ = node_allocator_traits::allocate(alloc_, 1);
        tail_ = node_allocator_traits::allocate(alloc_, 1);
        node_allocator_traits::construct(alloc_, head_, MAX_LEVEL);
        node_allocator_traits::construct(alloc_, tail_, MAX_LEVEL);

        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            head_->next[i] = tail_;
        }
        tail_->prev = head_;
        size_ = 0;
    }

    template <typename U>
    iterator insert_func(U&& value)
    {
        std::vector<skip_list_node*> update(MAX_LEVEL, nullptr);
        skip_list_node* current = head_;

        for (int i = MAX_LEVEL - 1; i >= 0; --i)
        {
            while (current->next[i] != tail_ && comp_(current->next[i]->value.value(), value))
            {
                current = current->next[i];
            }
            update[i] = current;
        }

        int new_level = random_level();
        skip_list_node* new_node = node_allocator_traits::allocate(alloc_, 1);
        node_allocator_traits::construct(alloc_, new_node, std::forward<U>(value), new_level);

        for (int i = 0; i < new_level; ++i)
        {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }

        new_node->prev = update[0];
        if (new_node->next[0])
        {
            new_node->next[0]->prev = new_node;
        }
        ++size_;
        return iterator(new_node);
    }

public:
    /**
     * @class iterator
     * @brief Bidirectional iterator for skip_list
     */
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() : current_(nullptr) {}
        explicit iterator(skip_list_node* node) : current_(node) {}

        /// @brief Dereference operator
        reference operator*() const {
            if (!current_->value.has_value())
            {
                throw std::runtime_error("Dereferencing invalid iterator");
            }
            return current_->value.value();
        }

        /// @brief Member access operator
        pointer operator->() const {
            return &(current_->value.value());
        }

        /// @brief Prefix increment
        iterator& operator++()
        {
            if (current_ == nullptr || !current_->next[0])
            {
                throw std::out_of_range("Incrementing past end");
            }
            current_ = current_->next[0];
            return *this;
        }

        /// @brief Postfix increment
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /// @brief Prefix decrement
        iterator& operator--()
        {
            if (current_ == nullptr || current_->prev == nullptr)
            {
                throw std::out_of_range("Decrementing before begin");
            }
            current_ = current_->prev;
            if (current_->prev == nullptr)   // Reached head_
            {
                throw std::out_of_range("Decrementing to before begin");
            }
            return *this;
        }

        /// @brief Postfix decrement
        iterator operator--(int)
        {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        /// @brief Equality comparison
        bool operator==(const iterator& other) const {
            return current_ == other.current_;
        }

        /// @brief Inequality comparison
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        skip_list_node* current_;
        friend class skip_list;
    };

    /**
     * @class const_iterator
     * @brief Constant bidirectional iterator for skip_list
     */
    class const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() : current_(nullptr) {}
        explicit const_iterator(skip_list_node* node) : current_(node) {}
        const_iterator(const iterator& it) : current_(it.current_) {}

        /// @brief Dereference operator
        reference operator*() const {
            if (!current_->value.has_value())
            {
                throw std::runtime_error("Dereferencing invalid iterator");
            }
            return current_->value.value();
        }

        /// @brief Member access operator
        pointer operator->() const {
            return &(current_->value.value());
        }

        /// @brief Prefix increment
        const_iterator& operator++()
        {
            if (current_ == nullptr || !current_->next[0])
            {
                throw std::out_of_range("Incrementing past end");
            }
            current_ = current_->next[0];
            return *this;
        }

        /// @brief Postfix increment
        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /// @brief Prefix decrement
        const_iterator& operator--()
        {
            if (current_ == nullptr || current_->prev == nullptr)
            {
                throw std::out_of_range("Decrementing before begin");
            }
            current_ = current_->prev;
            if (current_->prev == nullptr)   // Reached head_
            {
                throw std::out_of_range("Decrementing to before begin");
            }
            return *this;
        }

        /// @brief Postfix decrement
        const_iterator operator--(int)
        {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        /// @brief Equality comparison
        bool operator==(const const_iterator& other) const {
            return current_ == other.current_;
        }

        /// @brief Inequality comparison
        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        skip_list_node* current_;
        friend class skip_list;
    };

    /// @brief Default constructor
    skip_list(const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : comp_(comp), alloc_(alloc), head_(nullptr), tail_(nullptr), size_(0),
          gen_(std::random_device()()), dis_(0.0, 1.0)
    {
        initialize_sentinels();
    }

    /// @brief Constructor with allocator
    explicit skip_list(const Allocator& alloc)
        : comp_(Compare()), alloc_(alloc), head_(nullptr), tail_(nullptr), size_(0),
          gen_(std::random_device()()), dis_(0.0, 1.0)
    {
        initialize_sentinels();
    }

    /// @brief Copy constructor
    skip_list(const skip_list& other)
        : comp_(other.comp_), alloc_(node_allocator_traits::select_on_container_copy_construction(other.alloc_)),
          head_(nullptr), tail_(nullptr), size_(0), gen_(std::random_device()()), dis_(0.0, 1.0)
    {
        initialize_sentinels();
        for (const auto& val : other)
        {
            insert(val);
        }
    }

    /// @brief Move constructor
    skip_list(skip_list&& other) noexcept
        : comp_(std::move(other.comp_)), alloc_(std::move(other.alloc_)),
          head_(other.head_), tail_(other.tail_), size_(other.size_),
          gen_(std::move(other.gen_)), dis_(std::move(other.dis_))
    {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    /// @brief Destructor
    ~skip_list()
    {
        if (head_ && tail_)
        {
            clear();
            node_allocator_traits::destroy(alloc_, head_);
            node_allocator_traits::deallocate(alloc_, head_, 1);
            node_allocator_traits::destroy(alloc_, tail_);
            node_allocator_traits::deallocate(alloc_, tail_, 1);
        }
    }

    /// @brief Copy assignment operator
    skip_list& operator=(const skip_list& other)
    {
        if (this != &other)
        {
            clear();
            comp_ = other.comp_;
            if (node_allocator_traits::propagate_on_container_copy_assignment::value)
            {
                alloc_ = other.alloc_;
            }
            for (const auto& val : other)
            {
                insert(val);
            }
        }
        return *this;
    }

    /// @brief Move assignment operator
    skip_list& operator=(skip_list&& other) noexcept {
        if (this != &other)
        {
            clear();
            node_allocator_traits::destroy(alloc_, head_);
            node_allocator_traits::deallocate(alloc_, head_, 1);
            node_allocator_traits::destroy(alloc_, tail_);
            node_allocator_traits::deallocate(alloc_, tail_, 1);

            comp_ = std::move(other.comp_);
            alloc_ = std::move(other.alloc_);
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            gen_ = std::move(other.gen_);
            dis_ = std::move(other.dis_);

            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    /// @brief Returns the container's allocator
    Allocator get_allocator() const noexcept {
        return alloc_;
    }

    /// @brief Checks if container is empty
    bool empty() const noexcept {
        return size_ == 0;
    }

    /// @brief Returns number of elements
    std::size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Inserts an element
     *
     * @param value Value to insert
     * @return Iterator to the inserted element
     */
    iterator insert(const T& value)
    {
        return insert_func(value);
    }

    /**
     * @brief Inserts an element (move version)
     *
     * @param value Value to insert
     * @return Iterator to the inserted element
     */
    iterator insert(T&& value)
    {
        return insert_func(std::move(value));
    }

    /**
     * @brief Clears all elements from container
     */
    void clear()
    {
        skip_list_node* current = head_->next[0];
        while (current != tail_)
        {
            skip_list_node* next_node = current->next[0];
            node_allocator_traits::destroy(alloc_, current);
            node_allocator_traits::deallocate(alloc_, current, 1);
            current = next_node;
        }
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            head_->next[i] = tail_;
        }
        tail_->prev = head_;
        size_ = 0;
    }

    /// @brief Erases element at position
    iterator erase(iterator pos)
    {
        if (pos == end())
        {
            throw std::out_of_range("Cannot erase end iterator");
        }

        skip_list_node* node_to_erase = pos.current_;
        iterator next_it(node_to_erase->next[0]);

        std::vector<skip_list_node*> update(MAX_LEVEL, nullptr);
        skip_list_node* current = head_;

        for (int i = MAX_LEVEL - 1; i >= 0; --i)
        {
            while (current->next[i] != tail_ && comp_(current->next[i]->value.value(), node_to_erase->value.value()))
            {
                current = current->next[i];
            }
            update[i] = current;
        }

        for (int i = 0; i < node_to_erase->level; ++i)
        {
            if (update[i]->next[i] != node_to_erase)
            {
                break;
            }
            update[i]->next[i] = node_to_erase->next[i];
        }

        if (node_to_erase->next[0])
        {
            node_to_erase->next[0]->prev = node_to_erase->prev;
        }

        node_allocator_traits::destroy(alloc_, node_to_erase);
        node_allocator_traits::deallocate(alloc_, node_to_erase, 1);
        --size_;

        return next_it;
    }

    /**
     * @brief Swaps contents with another skip_list
     *
     * @param other Another skip_list to swap with
     */
    void swap(skip_list& other) noexcept
    {
        using std::swap;
        swap(head_, other.head_);
        swap(tail_, other.tail_);
        swap(size_, other.size_);
        swap(comp_, other.comp_);
        swap(alloc_, other.alloc_);
        swap(gen_, other.gen_);
        swap(dis_, other.dis_);
    }

    /// @brief Finds element with specific value
    iterator find(const T& value)
    {
        skip_list_node* current = head_;
        for (int i = MAX_LEVEL - 1; i >= 0; --i)
        {
            while (current->next[i] != tail_ && comp_(current->next[i]->value.value(), value))
            {
                current = current->next[i];
            }
        }
        current = current->next[0];
        if (current != tail_ && !comp_(value, current->value.value()))
        {
            return iterator(current);
        }
        return end();
    }

    /// @brief Finds element with specific value (const version)
    const_iterator find(const T& value) const {
        skip_list_node* current = head_;
        for (int i = MAX_LEVEL - 1; i >= 0; --i)
        {
            while (current->next[i] != tail_ && comp_(current->next[i]->value.value(), value))
            {
                current = current->next[i];
            }
        }
        current = current->next[0];
        if (current != tail_ && !comp_(value, current->value.value()))
        {
            return const_iterator(current);
        }
        return cend();
    }

    /**
     * @brief Checks if an element exists in the container
     *
     * @param value Value to search for
     * @return true if element exists, false otherwise
     */
    bool contains(const T& value) const {
        return find(value) != end();
    }

    /**
     * @brief Inserts an element using in-place construction
     *
     * @tparam Args Argument types for element construction
     * @param args Arguments to forward to element constructor
     * @return Iterator to the inserted element
     */
    template <typename... Args>
    iterator emplace(Args&&... args)
    {
        std::vector<skip_list_node*> update(MAX_LEVEL, nullptr);
        skip_list_node* current = head_;

        // Create temporary for comparison
        T temp(std::forward<Args>(args)...);

        for (int i = MAX_LEVEL - 1; i >= 0; --i)
        {
            while (current->next[i] != tail_ && comp_(current->next[i]->value.value(), temp))
            {
                current = current->next[i];
            }
            update[i] = current;
        }

        int new_level = random_level();
        skip_list_node* new_node = node_allocator_traits::allocate(alloc_, 1);
        node_allocator_traits::construct(alloc_, new_node, std::move(temp), new_level);

        for (int i = 0; i < new_level; ++i)
        {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }

        new_node->prev = update[0];
        if (new_node->next[0])
        {
            new_node->next[0]->prev = new_node;
        }
        ++size_;
        return iterator(new_node);
    }

    /**
     * @brief Inserts an element at the beginning of the container
     *
     * @param value Value to insert
     * @return Iterator to the inserted element
     */
    iterator push_front(const T& value)
    {
        return insert(value);
    }

    /**
     * @brief Inserts an element at the end of the container
     *
     * @param value Value to insert
     * @return Iterator to the inserted element
     */
    iterator push_back(const T& value)
    {
        return insert(value);
    }

    /**
     * @brief Removes the first element from the container
     */
    void pop_front()
    {
        if (empty())
        {
            throw std::out_of_range("pop_front called on empty skip_list");
        }
        erase(begin());
    }

    /**
     * @brief Removes the last element from the container
     */
    void pop_back()
    {
        if (empty())
        {
            throw std::out_of_range("pop_back called on empty skip_list");
        }
        erase(iterator(tail_->prev));
    }

    /**
     * @brief Resizes the container to contain count elements
     *
     * @param count New size of the container
     * @param value Value to initialize new elements with
     */
    void resize(std::size_t count, const T& value = T())
    {
        if (count > size_)
        {
            for (std::size_t i = size_; i < count; ++i)
            {
                insert(value);
            }
        }
        else if (count < size_)
        {
            auto it = begin();
            std::advance(it, count);
            while (it != end())
            {
                it = erase(it);
            }
        }
    }

    /// @brief Returns iterator to beginning
    iterator begin() noexcept {
        return iterator(head_->next[0]);
    }

    /// @brief Returns const iterator to beginning
    const_iterator begin() const noexcept {
        return const_iterator(head_->next[0]);
    }

    /// @brief Returns const iterator to beginning
    const_iterator cbegin() const noexcept {
        return const_iterator(head_->next[0]);
    }

    /// @brief Returns iterator to end
    iterator end() noexcept {
        return iterator(tail_);
    }

    /// @brief Returns const iterator to end
    const_iterator end() const noexcept {
        return const_iterator(tail_);
    }

    /// @brief Returns const iterator to end
    const_iterator cend() const noexcept {
        return const_iterator(tail_);
    }

    /// @brief Equality operator
    bool operator==(const skip_list& other) const {
        if (size_ != other.size_) return false;
        auto it1 = cbegin();
        auto it2 = other.cbegin();
        while (it1 != cend() && it2 != other.cend())
        {
            if (*it1 != *it2) return false;
            ++it1;
            ++it2;
        }
        return true;
    }

    /// @brief Inequality operator
    bool operator!=(const skip_list& other) const {
        return !(*this == other);
    }
};

#endif