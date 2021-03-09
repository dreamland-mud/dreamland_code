#ifndef ARGPARSERS_H
#define ARGPARSERS_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include "tao/pegtl.hpp"
#include "flagtable.h"
#include "flagtableregistry.h"
#include "exception.h"
#include "itemflags.h"
#include "pcharacter.h"
#include "merc.h"

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace TAO_PEGTL_NAMESPACE::mud
{
   struct ParseException : public Exception {
        ParseException( const char * fmt, ... ) throw();
        virtual ~ParseException();
   };

   template< typename Args, DLString Args::*Field >
   struct bind
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, Args& args )
      {
         args.*Field = in.string();
      }
   };

   template< typename Args, bitnumber_t Args::*Field, const FlagTable& FT >
   struct bind_flag
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, Args& args )
      {
         DLString input = in.string();
         bitnumber_t value = FT.value(input, false);

         if (value == NO_FLAG)
             throw ParseException("Значение %s не найдено в таблице %s.", 
                    input.c_str(), FlagTableRegistry::getName(&FT).c_str());

         args.*Field = value;
      }
   };

   template< typename Args, int Args::*Field, int LOW, int HIGH >
   struct bind_range
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, Args& args )
      {
         DLString input = in.string();
         int value = input.toInt();

         if (value < LOW) 
            throw ParseException("%d: значение не может быть меньше %d.", value, LOW);

         if (value > HIGH)
            throw ParseException("%d: значение не может быть больше %d.", value, HIGH);

         args.*Field = value;
      }
   };

    struct spaces : plus< space > {};
    struct word   : plus< alpha > {};
    struct number : plus< digit > {};

    struct wclass  : word {};
    struct level_value : number {};
    struct tier_value : number {}; 
    struct align_value: seq< opt<one<'-'> >, number > {};
    struct word_value : word {};

    struct args_wclass { bitnumber_t wclass; };
    struct args_level { int level; };
    struct args_tier { int tier; };
    struct args_word { DLString word; };
    struct args_align { int align; };

    template< typename Rule > struct action {};
    template<> struct action< level_value > : bind_range< args_level, &args_level::level, 0, 110 > {};
    template<> struct action< align_value > : bind_range< args_align, &args_align::align, ALIGN_EVIL, ALIGN_GOOD > {};
    template<> struct action< tier_value > : bind_range< args_tier, &args_tier::tier, 1, 5 > {};
    template<> struct action< wclass > : bind_flag< args_wclass, &args_wclass::wclass, weapon_class > {};
    template<> struct action< word_value > : bind< args_word, &args_word::word > {};
}

template< typename Grammar, typename Args > 
bool parse_input(PCharacter *ch, const DLString &_input, Args &args)
{
    DLString input(_input);

    if (input.stripWhiteSpace().empty()) {
        ch->pecho("Этой команде необходимы аргументы.");
        return false;
    }

    pegtl::memory_input in(input, "");

    try {
        pegtl::parse< Grammar, pegtl::mud::action >( in, args );
        return true;

    } catch (const pegtl::parse_error &pe) {
        using namespace std;
        ostringstream buf;    
        const auto p = pe.positions().front();

        buf << "Ошибка:" << endl 
            << in.line_at( p ) << endl
            << setw( p.column ) << "{R^{x" << endl;
         buf << pe.message() << endl;
        ch->send_to(buf);

    } 
    catch (const pegtl::mud::ParseException &pex) {
        ch->pecho(pex.what());
    }

    return false;
}

#endif

