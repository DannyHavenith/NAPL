#if !defined( EVENT_TO_CODES_HPP)
#define EVENT_TO_CODES_HPP
#include "midi_event_types.hpp"
#include "midi_event_visitor.hpp"

namespace events
{
    /// a midi event visitor that translates midi channel events to series of bytes.
    struct event_to_code_translator : public visitor< event_to_code_translator>
    {
        typedef unsigned char byte;

        event_to_code_translator()
            : length(0)
        {
        }

        using visitor< event_to_code_translator>::operator();

        void operator()( const note_on &event)
        {
            emit( 0x90, event.number, event.velocity);
        }

        void operator()( const note_off &event)
        {
            emit( 0x80, event.number, event.velocity);
        }

		void operator()( const note_aftertouch &event)
        {
            emit( 0xa0, event.number, event.velocity);
        }

		void operator()( const controller &event)
        {
            emit( 0xb0, event.which, event.value);
        }

		void operator()( const program_change &event)
        {
            emit( 0xc0, event.program);
        }

		void operator()( const channel_aftertouch &event)
        {
            emit( 0xd0, event.value);
        }

		void operator()( const pitch_bend &event)
        {
            emit( 0xe0, event.value & 0x0f, (event.value >> 8) & 0x0f);
        }

    public:
        byte codes[3];
        byte length;

    private:
        void emit( byte code, byte par1, byte par2)
        {
            length = 3;
            codes[0] = code | current_channel;
            codes[1] = par1;
            codes[2] = par2;
        }

        void emit( byte code, byte par1)
        {
            length = 2;
            codes[0] = code | current_channel;
            codes[1] = par1;
        }


    };
}
#endif //EVENT_TO_CODES_HPP