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

                virtual bool serialize(cyanvne::core::stream::OutStreamInterface& out) const = 0;
                virtual bool deserialize(cyanvne::core::stream::InStreamInterface& in) = 0;

                virtual ~BinarySerialiable() = default;
            };

            template <typename T>
            bool serialize_object(cyanvne::core::stream::OutStreamInterface& out, const T& value);

            template <typename T>
            bool deserialize_object(cyanvne::core::stream::InStreamInterface& in, T& value);

            namespace detail 
            {
                constexpr bool TARGET_ORDER_IS_BIG_ENDIAN = true;

                constexpr bool NATIVE_IS_LITTLE_ENDIAN = []() -> bool
                    {
                        union
                        {
                            std::uint16_t i;    // Test with a 16-bit integer.
                            std::uint8_t c[2];  // Treat its memory as two 8-bit characters.
                        }
                        u = { 0x0102U };        // Assign value 0x0102 (high byte 0x01, low byte 0x02)
                        return u.c[0] == 0x02;
                    }();

                constexpr bool SWAP_BYTES_REQUIRED = NATIVE_IS_LITTLE_ENDIAN;

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
                bool write_fundamental(cyanvne::core::stream::OutStreamInterface& out, const T& value) requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
                {
	                T val_to_write = maybe_swap_bytes(value);
                    return out.write(&val_to_write, sizeof(T)) == sizeof(T);
                }

                template <typename T>
                bool read_fundamental(cyanvne::core::stream::InStreamInterface& in, T& value) requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
                {
	                T temp_val;
	                if (in.read(&temp_val, sizeof(T)) != sizeof(T))
                    {
                        return false;
                    }
                    value = maybe_swap_bytes(temp_val);
                    return true;
                }
            }

            template <typename T> struct is_std_vector : std::false_type
            {  };
            template <typename Elem, typename Alloc> struct is_std_vector<std::vector<Elem, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_vector_v = is_std_vector<std::decay_t<T>>::value;

            template <typename T> struct is_std_map : std::false_type
            {  };
            template <typename Key, typename Value, typename Comp, typename Alloc> struct is_std_map<std::map<Key, Value, Comp, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_map_v = is_std_map<std::decay_t<T>>::value;

            template <typename T> struct is_std_set : std::false_type
            {  };
            template <typename Key, typename Comp, typename Alloc> struct is_std_set<std::set<Key, Comp, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_set_v = is_std_set<std::decay_t<T>>::value;

            template <typename T> struct is_std_unordered_map : std::false_type
            {  };
            template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Alloc> struct is_std_unordered_map<std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_unordered_map_v = is_std_unordered_map<std::decay_t<T>>::value;

            template <typename T> struct is_std_unordered_set : std::false_type
            {  };
            template <typename Key, typename Hash, typename KeyEqual, typename Alloc> struct is_std_unordered_set<std::unordered_set<Key, Hash, KeyEqual, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_unordered_set_v = is_std_unordered_set<std::decay_t<T>>::value;

            template <typename T> struct is_std_list : std::false_type
            {  };
            template <typename Elem, typename Alloc> struct is_std_list<std::list<Elem, Alloc>> : std::true_type
            {  };
            template <typename T> constexpr bool is_std_list_v = is_std_list<std::decay_t<T>>::value;

            template <typename T>
            bool serialize_object(cyanvne::core::stream::OutStreamInterface& out, const T& value)
            {
                using StrippedT = std::decay_t<T>; // remove & and cv

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
                    if (!detail::write_fundamental(out, len))
                    {
                        return false;
                    }
                    if (len > 0)
                    {
                        if (out.write(value.data(), len) != len)
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_vector_v<StrippedT> || is_std_list_v<StrippedT>)
                {
                    size_t size = value.size();
                    if (!detail::write_fundamental(out, size))
                    {
                        return false;
                    }
                    for (const auto& item : value)
                    {
                        if (!serialize_object(out, item))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT>)
                {
                    size_t size = value.size();
                    if (!detail::write_fundamental(out, size))
                    {
                        return false;
                    }
                    for (const auto& pair : value)
                    {
                        if (!serialize_object(out, pair.first))
                        {
                            return false;
                        }
                        if (!serialize_object(out, pair.second))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>)
                {
                    size_t size = value.size();
                    if (!detail::write_fundamental(out, size))
                    {
                        return false;
                    }
                    for (const auto& item : value)
                    {
                        if (!serialize_object(out, item))
                        {
                            return false;
                        }
                    }
                    return true;
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
                    return false;
                }
            }

            template <typename T>
            bool deserialize_object(cyanvne::core::stream::InStreamInterface& in, T& value)
            {
                using StrippedT = std::decay_t<T>;

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
                    if (!detail::read_fundamental(in, len))
                    {
                        return false;
                    }
                    try
                    {
                        value.resize(len);
                    }
                    catch (const std::bad_alloc&)
                    {
                        return false;
                    }
                    if (len > 0)
                    {
                        if (in.read(value.data(), len) != len)
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_vector_v<StrippedT>)
                {
                    size_t size;
                    if (!detail::read_fundamental(in, size))
                    {
                        return false;
                    }
                    if (size > value.max_size())
                    {
                        return false;
                    }

                    try
                    {
                        value.clear();
                        value.resize(size);
                    }
                    catch (const std::exception&)
                    {
                        return false;
                    }
                    for (size_t i = 0; i < size; ++i)
                    {
                        if (!deserialize_object(in, value[i]))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_list_v<StrippedT>)
                {
                    size_t size;
                    if (!detail::read_fundamental(in, size))
                    {
                        return false;
                    }
                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::value_type temp_item;
                        if (!deserialize_object(in, temp_item))
                        {
                            return false;
                        }
                        try
                        {
                            value.push_back(std::move(temp_item));
                        }
                        catch (const std::exception&)
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_map_v<StrippedT> || is_std_unordered_map_v<StrippedT>)
                {
                    size_t size;
                    if (!detail::read_fundamental(in, size))
                    {
                        return false;
                    }
                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::key_type k;
                        typename StrippedT::mapped_type v;
                        if (!deserialize_object(in, k))
                        {
                            return false;
                        }
                        if (!deserialize_object(in, v))
                        {
                            return false;
                        }
                        try
                        {
                            value.emplace(std::move(k), std::move(v));
                        }
                        catch (const std::exception&)
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_set_v<StrippedT> || is_std_unordered_set_v<StrippedT>)
                {
                    size_t size;
                    if (!detail::read_fundamental(in, size))
                    {
                        return false;
                    }
                    value.clear();
                    for (size_t i = 0; i < size; ++i)
                    {
                        typename StrippedT::key_type k;
                        if (!deserialize_object(in, k))
                        {
                            return false;
                        }
                        try
                        {
                            value.emplace(std::move(k));
                        }
                        catch (const std::exception&)
                        {
                            return false;
                        }
                    }
                    return true;
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
                    return false;
                }
            }

        }
    }
}