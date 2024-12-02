#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <optional>

namespace helpers
{
	template <typename T>
    std::optional<int> index_in_vector(const std::vector<T> vec, const T in)
	{
        auto it = std::ranges::find(vec, in);
        if (it != vec.end()) {
            return std::distance(vec.begin(), it);
        }

        // Return std::nullopt if not found
        return std::nullopt;
	}

    template <typename T>
    bool exists_in_vector(const std::vector<T> vec, const T in)
    {
        return std::ranges::any_of(vec, [in](const T& val)
            { return in == val; }
        );
    }
}