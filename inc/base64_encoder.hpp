/**
 * @author Alejandro Solozabal
 *
 * @file base64_encoder.hpp
 *
 */

#ifndef BASE64_ENCODER_H_
#define BASE64_ENCODER_H_

#include <string>
#include <openssl/pem.h>

class Base64Encoder
{
    public:
        Base64Encoder()
        {
            /* Initialize base64 filter and memory sink BIO */
            m_b64_bio = BIO_new(BIO_f_base64());
            m_mem_bio = BIO_new(BIO_s_mem());

            /* Link the BIOs by creating a filter-sink BIO chain */
            BIO_push(m_b64_bio, m_mem_bio);

            /* No newlines every 64 characters or less */
            BIO_set_flags(m_b64_bio, BIO_FLAGS_BASE64_NO_NL);

            /* Store address of mem_bio's memory structure */
            BIO_get_mem_ptr(m_mem_bio, &m_mem_buffer);
        }

        std::string& Encode(std::string input)
        {
            /* Reset bio structures */
            BIO_reset(m_b64_bio);
            BIO_reset(m_mem_bio);

            /* Records base64 encoded data */
            BIO_write(m_b64_bio, input.data(), input.length());

            /* Flush data. Necessary for b64 encoding, because of pad characters */
            BIO_flush(m_b64_bio);

            /* Add null terminator */
            BUF_MEM_grow(m_mem_buffer, (*m_mem_buffer).length + 1);
            (*m_mem_buffer).data[(*m_mem_buffer).length] = '\0';

            m_output = (*m_mem_buffer).data;

            return m_output;
        }

        ~Base64Encoder()
        {
            BIO_free_all(m_b64_bio);
        }

    private:
        BIO *m_b64_bio = nullptr, *m_mem_bio = nullptr;
        BUF_MEM *m_mem_buffer = nullptr;
        std::string m_output;
};

#endif /* BASE64_ENCODER_H_ */