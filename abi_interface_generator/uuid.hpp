#pragma once

#include <string>
#include <string_view>
#include <type_traits>

namespace glasssix
{
	/// <summary>
	/// A cross-platform UUID library.
	/// </summary>
	class uuid
	{
	public:
		using storage_type = std::aligned_storage_t<16>;
		
		uuid() noexcept;
		uuid(const uuid& other) noexcept;
		uuid(uuid&& other) noexcept;
		explicit uuid(std::string_view value);
		virtual ~uuid();
		uuid& operator=(const uuid& right) noexcept;
		uuid& operator=(uuid&& right) noexcept;
		operator std::string() const;
		std::string to_string() const;
		static uuid generate();
		friend bool operator==(const uuid& left, const uuid& right) noexcept;
		friend bool operator!=(const uuid& left, const uuid& right) noexcept;

	private:
		storage_type storage_;
	};
}
