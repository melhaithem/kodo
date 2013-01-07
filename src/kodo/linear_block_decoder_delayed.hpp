// Copyright Steinwurf ApS 2011-2012.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#ifndef KODO_LINEAR_BLOCK_DECODER_DELAYED_HPP
#define KODO_LINEAR_BLOCK_DECODER_DELAYED_HPP

#include <stdint.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>

#include "linear_block_vector.hpp"

namespace kodo
{
    template<class SuperCoder>
    class linear_block_decoder_delayed : public SuperCoder
    {
    public:

        /// The field we use
        typedef typename SuperCoder::field_type field_type;

        /// The value_type used to store the field elements
        typedef typename field_type::value_type value_type;

    public:

        /// The decode function which consumes an incomming symbol and
        /// the corresponding symbol_id
        /// @param symbol_data the encoded symbol
        /// @param symbol_id the coefficients used to create the encoded symbol
        void decode(uint8_t *symbol_data, uint8_t *symbol_id)
            {
                assert(symbol_data != 0);
                assert(symbol_id != 0);

                value_type *symbol
                    = reinterpret_cast<value_type*>(symbol_data);

                value_type *vector
                    = reinterpret_cast<value_type*>(symbol_id);

                decode_with_vector(vector, symbol);
            }

        /// decode raw takes systematic packets inserts them into the decoder
        /// for this specific coder no backwards substitution are performed
        /// until the decoder reaches full rank.
        void decode_raw(const uint8_t *symbol_data, uint32_t symbol_index)
            {
                assert(symbol_index < SuperCoder::symbols());
                assert(symbol_data != 0);

                if(m_uncoded[symbol_index])
                    return;

                const value_type *symbol
                    = reinterpret_cast<const value_type*>( symbol_data );

                if(m_coded[symbol_index])
                {
                    SuperCoder::swap_decode(symbol_index, symbol);
                }
                else
                {
                    // Stores the symbol and updates the corresponding
                    // encoding vector
                    SuperCoder::store_uncoded_symbol(symbol_index, symbol);

                    // We have increased the rank
                    ++m_rank;

                    m_uncoded[ symbol_index ] = true;

                    if(symbol_index > m_maximum_pivot)
                    {
                        m_maximum_pivot = symbol_index;
                    }

                }

                if(SuperCoder::is_complete())
                {
                    final_backward_substitute();
                }

            }


    protected:

        // Fetch the variables needed
        using SuperCoder::m_rank;
        using SuperCoder::m_maximum_pivot;
        using SuperCoder::m_coded;
        using SuperCoder::m_uncoded;

    protected:

        /// Decodes a symbol based on the vector
        /// @param symbol_data buffer containing the encoding symbol
        /// @param vector_data buffer containing the encoding vector
        void decode_with_vector(value_type *vector_data, value_type *symbol_data)
            {
                assert(symbol_data != 0);
                assert(vector_data != 0);

                // See if we can find a pivot
                boost::optional<uint32_t> pivot_id
                    = SuperCoder::forward_substitute_to_pivot(vector_data,
                                                              symbol_data);

                if(!pivot_id)
                    return;

                if(!fifi::is_binary<field_type>::value)
                {
                    // Normalize symbol and vector
                    SuperCoder::normalize(*pivot_id, vector_data, symbol_data);
                }

                // Now save the received symbol
                SuperCoder::store_coded_symbol(*pivot_id, vector_data,
                                               symbol_data);

                // We have increased the rank
                ++m_rank;

                m_coded[ *pivot_id ] = true;

                if(*pivot_id > m_maximum_pivot)
                {
                    m_maximum_pivot = *pivot_id;
                }

                if(SuperCoder::is_complete())
                {
                    final_backward_substitute();
                }
            }

    protected:

        void final_backward_substitute()
            {
                assert(SuperCoder::is_complete());

                uint32_t symbols = SuperCoder::symbols();

                for(uint32_t i = symbols; i --> 0;)
                {
                    value_type *vector_i = SuperCoder::vector(i);
                    value_type *symbol_i = reinterpret_cast<value_type*>(SuperCoder::symbol(i));

                    SuperCoder::backward_substitute(
                        i, vector_i, symbol_i);
                }
            }
    };
}

#endif

