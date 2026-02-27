#pragma once

#include <cstddef>
#include <cstdlib>

class ArenaAllocator {
public:
				inline explicit ArenaAllocator(size_t bytes) : m_size(bytes) {
								m_buffer = static_cast<std::byte*>(malloc(m_size));
								m_offset = m_buffer;
				};

				template<typename T>
								T* alloc() {
												void* ptr = m_offset;
												m_offset += sizeof(T);
												return new (ptr) T();
								}

				inline ArenaAllocator(const ArenaAllocator& other) = delete;
				inline void operator = (const ArenaAllocator& other) = delete;

				~ArenaAllocator() {
								free(m_buffer);
				}
private:
				size_t m_size;
				std::byte* m_buffer;
				std::byte* m_offset;
};
