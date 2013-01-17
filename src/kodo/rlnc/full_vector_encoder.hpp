// Copyright Steinwurf ApS 2011-2012.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#ifndef KODO_FULL_VECTOR_ENCODER_HPP
#define KODO_FULL_VECTOR_ENCODER_HPP

#include <stdint.h>

#include "../linear_block_vector.hpp"

namespace kodo
{
    /// The full vector RLNC encoder.
    /// This type of encoder produces and sends the entire encoding vector.
    /// Sending the entire encoding vector has the advantage that intermediate
    /// nodes may easily perform recoding (a special operation used in Network
    /// Coding algorithms).
    template<class SuperCoder>
    class full_vector_encoder : public SuperCoder
    {
    public:

        /// The field type
        typedef typename SuperCoder::field_type field_type;

        /// The value type
        typedef typename field_type::value_type value_type;

        /// The encoding vector
        typedef linear_block_vector<field_type> vector_type;

        /// Pointer to coder produced by the factories
        typedef typename SuperCoder::pointer pointer;

    public:

        /// The factory layer associated with this coder.
        /// Maintains the block generator needed for the encoding vectors.
        class factory : public SuperCoder::factory
        {
        public:

            /// @copydoc final_coder_factory::factory::factory()
            factory(uint32_t max_symbols, uint32_t max_symbol_size)
                : SuperCoder::factory(max_symbols, max_symbol_size)
                { }


            /// @return the required payload buffer size in bytes
            uint32_t max_symbol_id_size() const
                {
                    uint32_t max_symbol_id_size =
                        vector_type::size( SuperCoder::factory::max_symbols() );

                    assert(max_symbol_id_size > 0);

                    return max_symbol_id_size;
                }
        };

    public:

        /// @copydoc final_coder_factory::initialize()
        void initialize(uint32_t symbols, uint32_t symbol_size)
            {
                SuperCoder::initialize(symbols, symbol_size);

                m_symbol_id_size = vector_type::size( symbols );
                assert(m_symbol_id_size > 0);

                m_count = 0;
            }

        /// @copydoc linear_block_encoder::encode_with_vector()
        /// @return amount of used buffer in bytes
        uint32_t encode(uint8_t *symbol_data, uint8_t *symbol_id)
            {
                assert(symbol_data != 0);
                assert(symbol_id != 0);

                // Get the data for the encoding vector
                value_type *vector
                    = reinterpret_cast<value_type*>(symbol_id);

                // Fill the encoding vector
                SuperCoder::generate(m_count, vector);
                ++m_count;

                SuperCoder::encode(symbol_data, symbol_id);

                return m_symbol_id_size;
            }


        /// @return the required payload buffer size in bytes
        uint32_t symbol_id_size() const
            {
                return m_symbol_id_size;
            }

    protected:

        /// The size of the encoding vector in bytes
        uint32_t m_symbol_id_size;

        /// Keeping track of the number of packets sent
        uint32_t m_count;
    };
}

#endif
