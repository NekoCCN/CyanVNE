#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <Core/Stream/Stream.h>

namespace cyanvne
{
    namespace core
    {
        namespace binaryserializer
        {
            class BinarySerialiable
            {
            protected:
                BinarySerialiable() = default;
            public:
                BinarySerialiable(const BinarySerialiable&) = delete;
                BinarySerialiable& operator=(const BinarySerialiable&) = delete;
                BinarySerialiable(BinarySerialiable&&) = delete;
                BinarySerialiable& operator=(BinarySerialiable&&) = delete;

                virtual std::ptrdiff_t serialize(cyanvne::core::stream::OutStreamInterface& out) const = 0;
                virtual std::ptrdiff_t deserialize(cyanvne::core::stream::InStreamInterface& in) = 0;

                virtual ~BinarySerialiable() = default;
            };

            template <typename T>
            std::ptrdiff_t serialize_object(cyanvne::core::stream::OutStreamInterface& out, const T& value);

            template <typename T>
            std::ptrdiff_t deserialize_object(cyanvne::core::stream::InStreamInterface& in, T& value);

            namespace detail
            {
                constexpr bool TARGET_ORDER_IS_BIG_ENDIAN = true;

                const bool NATIVE_IS_LITTLE_ENDIAN = []() -> bool
                    {
                        union
                        {
                            std::uint16_t i;
                            std::uint8_t c[2];
                        }
                        u = { 0x0102U };
                        return u.c[0] == 0x02;
                    }();

                const bool SWAP_BYTES_REQUIRED = NATIVE_IS_LITTLE_ENDIAN;

                template<typename T>
                T maybe_swap_bytes(T val)
                {
                    static_assert(std::is_fundamental_v<T> || std::is_enum_v<T>,
                        "Byte swapping is supported only for primitive types or enumeration types.");
                    if constexpr (SWAP_BYTES_REQUIRED && sizeof(T) > 1)
                    {
                        char* bytes = reinterpret_cast<char*>(&val);
                        std::reverse(bytes, bytes + sizeof(T));
                    }
                    return val;
                }

                template <typename T>
                std::ptrdiff_t write_fundamental(cyanvne::core::stream::OutStreamInterface& out, const T& value) requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
                {
                    T val_to_write = maybe_swap_bytes(value);
                    if (out.write(&val_to_write, sizeof(T)) == sizeof(T))
                    {
                        return static_cast<std::ptrdiff_t>(sizeof(T));
                    }
                    return -1;
                }

                template <typename T>
                std::ptrdiff_t read_fundamental(cyanvne::core::stream::InStreamInterface& in, T& value) requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
                {
                    T temp_val;
                    if (in.read(&temp_val, sizeof(T)) == sizeof(T))
                    {
                        value = maybe_swap_bytes(temp_val);
                        return static_cast<std::ptrdiff_t>(sizeof(T));
                    }
                    return -1;
                }
            }

            template <typename T> struct is_std_vector : std::false_type
            {  };
            template <typename Elem, typename Alloc> struct is_std_vector<std::vector<Elem, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_vector_v = is_std_vector<std::decay_t<T>>::value;

            template <typename T> struct is_std_map : std::false_type
            {  };
            template <typename Key, typename Value, typename Comp, typename Alloc> struct is_std_map<std::map<Key, Value, Comp, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_map_v = is_std_map<std::decay_t<T>>::value;

            template <typename T> struct is_std_set : std::false_type
            {  };
            template <typename Key, typename Comp, typename Alloc> struct is_std_set<std::set<Key, Comp, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_set_v = is_std_set<std::decay_t<T>>::value;

            template <typename T> struct is_std_unordered_map : std::false_type
            {  };
            template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Alloc> struct is_std_unordered_map<std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_unordered_map_v = is_std_unordered_map<std::decay_t<T>>::value;

            template <typename T> struct is_std_unordered_set : std::false_type
            {  };
            template <typename Key, typename Hash, typename KeyEqual, typename Alloc> struct is_std_unordered_set<std::unordered_set<Key, Hash, KeyEqual, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_unordered_set_v = is_std_unordered_set<std::decay_t<T>>::value;

            template <typename T> struct is_std_list : std::false_type
            {  };
            template <typename Elem, typename Alloc> struct is_std_list<std::list<Elem, Alloc>> : std::true_type {};
            template <typename T> constexpr bool is_std_list_v = is_std_list<std::decay_t<T>>::value;


            template <typename T>
            std::ptrdiff_t serialize_object(cyanvne::core::stream::OutStreamInterface& out, const T& value)
            {
                using StrippedT = std::decay_t<T>;
                std::ptrdiff_t total_bytes_written = 0;
                std::ptrdiff_t bytes_written;

                if constexpr (std::is_base_of_v<BinarySerialiable, StrippedT>)
                {
                    return value.serialize(out);
                }
                else if constexpr (std::is_fundamental_v<StrippedT> || std::is_enum_v<StrippedT>)
                {
                    return detail::write_fundamental(out, value);
                }
                else if constexpr (std::is_same_v<StrippedT, std::string>)
                {
                    size_t len = value.length();
                    bytes_written = detail::write_fundamental(out, len);
                    if (bytes_written == -1)
                    {
                        return -1;
                    }
                    total_bytes_written += bytes_written;

                    if (len > 0)
                    {
                        size_t data_bytes_written = out.write(value.data(), len);
                        if (data_bytes_written != len)
                        {
                            return -1;
                        }
                        total_bytes_written += static_cast<std::ptrdiff_t>(data_bytes_written);
                    }
                    return total_bytes_written;
                }
                else if constexpr (is_std_vector_v<StrippedT> || is_std_list_v<StrippedT>)
                {
                    size_t size = value.size();
                    bytes_written = detail::write_fundamental(out, size);
                    if (bytes_written == -1)
                    {
                        return -1;
                    }
                    total_bytes_written += bytes_written;

                    for (const auto& item : value)
                    {
                        bytes_written = serialize_object(out, item);
                        if (bytes_written == -1)
                        {
                            return -1;
                        }
                        total_bytes_written += bytes_written;
                    }
                    return total_bytes_written;
                }
                else if constexpr (is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT>)
                {
                    size_t size = value.size();
                    bytes_written = detail::write_fundamental(out, size);
                    if (bytes_written == -1)
                    {
                        return -1;
                    }
                    total_bytes_written += bytes_written;

                    for (const auto& pair : value)
                    {
                        bytes_written = serialize_object(out, pair.first);
                        if (bytes_written == -1)
                        {
                            return -1;
                        }
                        total_bytes_written += bytes_written;

                        bytes_written = serialize_object(out, pair.second);
                        if (bytes_written == -1)
                        {
                            return -1;
                        }
                        total_bytes_written += bytes_written;
                    }
                    return total_bytes_written;
                }
                else if constexpr (is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>)
                {
                    size_t size = value.size();
                    bytes_written = detail::write_fundamental(out, size);
                    if (bytes_written == -1)
                    {
                        return -1;
                    }
                    total_bytes_written += bytes_written;

                    for (const auto& item : value)
                    {
                        bytes_written = serialize_object(out, item);
                        if (bytes_written == -1)
                        {
                            return -1;
                        }
                        total_bytes_written += bytes_written;
                    }
                    return total_bytes_written;
                }
                else
                {
                    static_assert(std::is_base_of_v<BinarySerialiable, StrippedT> ||
                        (std::is_fundamental_v<StrippedT> || std::is_enum_v<StrippedT>) ||
                        std::is_same_v<StrippedT, std::string> ||
                        is_std_vector_v<StrippedT> || is_std_list_v<StrippedT> ||
                        is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT> ||
                        is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>,
                        "BinarySerializer: Unsupported type for serialize_object. "
                        "Type must be fundamental, std::string, std::vector, std::list, std::map, std::set, std::unordered_map, std::unordered_set, "
                        "or implement the BinarySerialiable interface.");
                    return -1;
                }
            }

            template <typename T>
            std::ptrdiff_t deserialize_object(cyanvne::core::stream::InStreamInterface& in, T& value)
            {
                using StrippedT = std::decay_t<T>;
                std::ptrdiff_t total_bytes_read = 0;
                std::ptrdiff_t bytes_read;

                if constexpr (std::is_base_of_v<BinarySerialiable, StrippedT>)
                {
                    return value.deserialize(in);
                }
                else if constexpr (std::is_fundamental_v<StrippedT> || std::is_enum_v<StrippedT>)
                {
                    return detail::read_fundamental(in, value);
                }
                else if constexpr (std::is_same_v<StrippedT, std::string>)
                {
                    size_t len;
                    bytes_read = detail::read_fundamental(in, len);
                    if (bytes_read == -1) return -1;
                    total_bytes_read += bytes_read;

                    try
                    {
                        value.resize(len);
                    }
                    catch (const std::exception&)
                    {
                        return -1;
                    }

                    if (len > 0)
                    {
                        size_t data_bytes_read = in.read(value.data(), len);
                        if (data_bytes_read != len) return -1;
                        total_bytes_read += static_cast<std::ptrdiff_t>(data_bytes_read);
                    }
                    return total_bytes_read;
                }
                else if constexpr (is_std_vector_v<StrippedT>)
                {
                    size_t size;
                    bytes_read = detail::read_fundamental(in, size);
                    if (bytes_read == -1) return -1;
                    total_bytes_read += bytes_read;

                    if (size > value.max_size())
                    {
                        return -1;
                    }
                    try
                    {
                        value.clear();
                        value.resize(size);
                    }
                    catch (const std::exception&)
                    {
                        return -1;
                    }

                    for (size_t i = 0; i < size; ++i)
                    {
                        bytes_read = deserialize_object(in, value[i]);
                        if (bytes_read == -1)
                        {
                            return -1;
                        }
                        total_bytes_read += bytes_read;
                    }
                    return total_bytes_read;
                }
                else if constexpr (is_std_list_v<StrippedT>)
                {
                    size_t size;
                    bytes_read = detail::read_fundamental(in, size);
                    if (bytes_read == -1)
                    {
                        return -1;
                    }
                    total_bytes_read += bytes_read;

                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::value_type temp_item;
                        bytes_read = deserialize_object(in, temp_item);
                        if (bytes_read == -1) return -1;
                        total_bytes_read += bytes_read;
                        try
                        {
                            value.push_back(std::move(temp_item)); // Potential bad_alloc
                        }
                        catch (const std::exception&)
                        {
                            return -1;
                        }
                    }
                    return total_bytes_read;
                }
                else if constexpr (is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT>)
                {
                    size_t size;
                    bytes_read = detail::read_fundamental(in, size);
                    if (bytes_read == -1)
                    {
                        return -1;
                    }
                    total_bytes_read += bytes_read;

                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::key_type k;
                        typename StrippedT::mapped_type v;

                        bytes_read = deserialize_object(in, k);
                        if (bytes_read == -1)
                        {
                            return -1;
                        }
                        total_bytes_read += bytes_read;

                        bytes_read = deserialize_object(in, v);
                        if (bytes_read == -1) return -1;
                        total_bytes_read += bytes_read;
                        try
                        {
                            value.emplace(std::move(k), std::move(v));
                        }
                        catch (const std::exception&)
                        {
                            return -1;
                        }
                    }
                    return total_bytes_read;
                }
                else if constexpr (is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>)
                {
                    size_t size;
                    bytes_read = detail::read_fundamental(in, size);
                    if (bytes_read == -1)
                    {
                        return -1;
                    }
                    total_bytes_read += bytes_read;

                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::key_type k;
                        bytes_read = deserialize_object(in, k);
                        if (bytes_read == -1)
                        {
                            return -1;
                        }
                        total_bytes_read += bytes_read;
                        try
                        {
                            value.emplace(std::move(k));
                        }
                        catch (const std::exception&)
                        {
                            return -1;
                        }
                    }
                    return total_bytes_read;
                }
                else
                {
                    static_assert(std::is_base_of_v<BinarySerialiable, StrippedT> ||
                        (std::is_fundamental_v<StrippedT> || std::is_enum_v<StrippedT>) ||
                        std::is_same_v<StrippedT, std::string> ||
                        is_std_vector_v<StrippedT> || is_std_list_v<StrippedT> ||
                        is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT> ||
                        is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>,
                        "BinarySerializer: Unsupported type for deserialize_object. "
                        "Type must be fundamental, std::string, std::vector, std::list, std::map, std::set, std::unordered_map, std::unordered_set, "
                        "or implement the BinarySerialiable interface.");
                    return -1;
                }
           }
        }
    }
}