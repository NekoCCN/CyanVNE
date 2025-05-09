#pragma once
#include <vector>
#include <string>
#include <map>
#include <type_traits>
#include <cstddef>
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

            template <typename T> struct is_std_vector : std::false_type
            {  };
            template <typename Elem, typename Alloc>
            struct is_std_vector<std::vector<Elem, Alloc>> : std::true_type
            {  };
            template <typename T>
            constexpr bool is_std_vector_v = is_std_vector<std::decay_t<T>>::value;

            template <typename T> struct is_std_map : std::false_type
            {  };
            template <typename Key, typename Value, typename Comp, typename Alloc>
            struct is_std_map<std::map<Key, Value, Comp, Alloc>> : std::true_type
            {  };
            template <typename T>
            constexpr bool is_std_map_v = is_std_map<std::decay_t<T>>::value;


            template <typename T>
            bool serialize_object(cyanvne::core::stream::OutStreamInterface& out, const T& value)
            {
                using StrippedT = std::decay_t<T>;

                if constexpr (std::is_base_of_v<BinarySerialiable, StrippedT>)
                {
                    return value.serialize(out);
                }
                else if constexpr (std::is_fundamental_v<StrippedT>)
                {
                    return out.write(&value, sizeof(StrippedT));
                }
                else if constexpr (std::is_same_v<StrippedT, std::string>)
                {
                    size_t len = value.length();
                    if (!out.write(&len, sizeof(len)))
                    {
                        return false;
                    }
                    if (len > 0)
                    {
                        if (!out.write(value.data(), len))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_vector_v<StrippedT>)
                {
                    size_t size = value.size();
                    if (!out.write(&size, sizeof(size)))
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
                else if constexpr (is_std_map_v<StrippedT>)
                {
                    size_t size = value.size();
                    if (!out.write(&size, sizeof(size)))
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
                else
                {
                    static_assert(std::is_base_of_v<BinarySerialiable, StrippedT> ||
                        std::is_fundamental_v<StrippedT> ||
                        std::is_same_v<StrippedT, std::string> ||
                        is_std_vector_v<StrippedT> ||
                        is_std_map_v<StrippedT>,
                        "BinarySerializer: Unsupported type for serialize_object. "
                        "Type must be fundamental, std::string, std::vector, std::map, "
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
                else if constexpr (std::is_fundamental_v<StrippedT>)
                {
                    return in.read(&value, sizeof(StrippedT));
                }
                else if constexpr (std::is_same_v<StrippedT, std::string>)
                {
                    size_t len;
                    if (!in.read(&len, sizeof(len)))
                    {
                        return false;
                    }
                    value.resize(len);
                    if (len > 0)
                    {
                        if (!in.read(&value[0], len))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_vector_v<StrippedT>)
                {
                    size_t size;
                    if (!in.read(&size, sizeof(size)))
                    {
                        return false;
                    }

                    value.clear();
                    value.resize(size);
                    for (size_t i = 0; i < size; ++i)
                    {
                        if (!deserialize_object(in, value[i]))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else if constexpr (is_std_map_v<StrippedT>)
                {
                    size_t size;
                    if (!in.read(&size, sizeof(size)))
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
                        value.emplace(std::move(k), std::move(v));
                    }
                    return true;
                }
                else
                {
                    static_assert(std::is_base_of_v<BinarySerialiable, StrippedT> ||
                        std::is_fundamental_v<StrippedT> ||
                        std::is_same_v<StrippedT, std::string> ||
                        is_std_vector_v<StrippedT> ||
                        is_std_map_v<StrippedT>,
                        "BinarySerializer: Unsupported type for deserialize_object. "
                        "Type must be fundamental, std::string, std::vector, std::map, "
                        "or implement the BinarySerialiable interface.");
                    return false;
                }
            }
        }
    }
}