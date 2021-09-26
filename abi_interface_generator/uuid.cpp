#include "uuid.hpp"
#include "fundamental_error.hpp"

#include <new>
#include <memory>
#include <utility>

#ifdef _WIN32
#include <Rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#else
#include <uuid/uuid.h>
#endif

namespace glasssix
{
	namespace
	{
#ifdef _WIN32
		template<typename... Args>
		void make_uuid_data(uuid::storage_type& storage, Args&&... args) noexcept
		{
			new (&storage) UUID{ std::forward<Args>(args)... };
		}

		UUID* get_underlying_data(uuid::storage_type& storage) noexcept
		{
			return std::launder(reinterpret_cast<UUID*>(&storage));
		}

		const UUID* get_underlying_data(const uuid::storage_type& storage) noexcept
		{
			return std::launder(reinterpret_cast<const UUID*>(&storage));
		}

		void destruct_uuid_data(uuid::storage_type& storage) noexcept
		{
			std::launder(reinterpret_cast<UUID*>(&storage))->~UUID();
		}
#else
		struct uuid_t_bag
		{
			uuid_t value;
		};

		template<typename... Args>
		void make_uuid_data(uuid::storage_type& storage, Args&&... args) noexcept
		{
			new (&storage) uuid_t_bag{ std::forward<Args>(args)... };
		}

		uuid_t_bag* get_underlying_data(uuid::storage_type& storage) noexcept
		{
			return std::launder(reinterpret_cast<uuid_t_bag*>(&storage));
		}

		const uuid_t_bag* get_underlying_data(const uuid::storage_type& storage) noexcept
		{
			return std::launder(reinterpret_cast<const uuid_t_bag*>(&storage));
		}

		void destruct_uuid_data(uuid::storage_type& storage) noexcept
		{
			std::launder(reinterpret_cast<uuid_t_bag*>(&storage))->~uuid_t_bag();
		}
#endif
	}

	uuid::uuid() noexcept
	{
		make_uuid_data(storage_);
	}

	uuid::uuid(const uuid& other) noexcept
	{
		make_uuid_data(storage_, *get_underlying_data(other.storage_));
	}

	uuid::uuid(uuid&& other) noexcept
	{
		make_uuid_data(storage_, std::exchange(*get_underlying_data(other.storage_), {}));
	}

	uuid::uuid(std::string_view value) : uuid{}
	{
#ifdef _WIN32
		if (UuidFromStringA(reinterpret_cast<RPC_CSTR>(const_cast<char*>(value.data())), get_underlying_data(storage_)) != RPC_S_OK)
#else
		if (uuid_parse(const_cast<char*>(value.data()), get_underlying_data(storage_)->value) != 0)
#endif
		{
			throw fundamental_error{ u8"Cannot create a UUID from a string in invalid format." };
		}
	}

	uuid::~uuid()
	{
		destruct_uuid_data(storage_);
	}

	uuid& uuid::operator=(const uuid& right) noexcept
	{
		return (destruct_uuid_data(storage_),  make_uuid_data(storage_, *get_underlying_data(right.storage_)), *this);
	}

	uuid& uuid::operator=(uuid&& right) noexcept
	{
		return (destruct_uuid_data(storage_), make_uuid_data(storage_, std::exchange(*get_underlying_data(right.storage_), {})), *this);
	}

	uuid::operator std::string() const
	{
		return to_string();
	}

	std::string uuid::to_string() const
	{
#ifdef _WIN32
		RPC_CSTR str{};

		if (UuidToStringA(get_underlying_data(storage_), &str) != RPC_S_OK)
		{
			throw fundamental_error{ u8"Cannot convert a UUID to a string because of invocation failure to \"UuidToString\"." };
		}

		std::unique_ptr<std::remove_pointer_t<RPC_CSTR>, void(*)(RPC_CSTR)> result{ str, [](RPC_CSTR inner) { RpcStringFreeA(&inner); } };

		return std::string(reinterpret_cast<char*>(result.get()));
#else
		std::string result(36, '\0');

		return (uuid_unparse(get_underlying_data(storage_)->value, result.data()), result);
#endif
	}

	uuid uuid::generate()
	{
		uuid result;

#ifdef _WIN32
		if (auto status = UuidCreate(get_underlying_data(result.storage_)); status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
		{
			throw fundamental_error{ u8"Cannot generate a UUID because of invocation failure to \"UuidCreate\"." };
		}
#else
		uuid_generate(get_underlying_data(result.storage_)->value);
#endif
		return result;
	}

	bool operator==(const uuid& left, const uuid& right) noexcept
	{
#ifdef _WIN32
		RPC_STATUS status{};

		return UuidCompare(get_underlying_data(const_cast<uuid::storage_type&>(left.storage_)), get_underlying_data(const_cast<uuid::storage_type&>(right.storage_)), &status) == 0;
#else
		return uuid_compare(get_underlying_data(left.storage_)->value, get_underlying_data(right.storage_)->value) == 0;
#endif
	}

	bool operator!=(const uuid& left, const uuid& right) noexcept
	{
		return !(left == right);
	}
}
