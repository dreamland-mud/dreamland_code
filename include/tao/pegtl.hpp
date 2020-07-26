/*

Welcome to the Parsing Expression Grammar Template Library (PEGTL).
-e See https://github.com/taocpp/PEGTL/ for more information, documentation, etc.

-e The library is licensed as follows:

The MIT License (MIT)

Copyright (c) 2007-2020 Dr. Colin Hirsch and Daniel Frey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
-e 
*/

#line 1 "amalgamated.hpp"
#line 1 "<built-in>"
#line 1 "<command-line>"
#line 1 "amalgamated.hpp"
#line 1 "tao/pegtl.hpp"
       
#line 1 "tao/pegtl.hpp"



#ifndef TAO_PEGTL_HPP
#define TAO_PEGTL_HPP

#line 1 "tao/pegtl/config.hpp"
       
#line 1 "tao/pegtl/config.hpp"



#ifndef TAO_PEGTL_CONFIG_HPP
#define TAO_PEGTL_CONFIG_HPP

#if !defined( TAO_PEGTL_NAMESPACE )
#define TAO_PEGTL_NAMESPACE tao::pegtl
#endif

#endif
#line 8 "tao/pegtl.hpp"
#line 1 "tao/pegtl/parse.hpp"
       
#line 1 "tao/pegtl/parse.hpp"



#ifndef TAO_PEGTL_PARSE_HPP
#define TAO_PEGTL_PARSE_HPP

#include <cassert>

#line 1 "tao/pegtl/apply_mode.hpp"
       
#line 1 "tao/pegtl/apply_mode.hpp"



#ifndef TAO_PEGTL_APPLY_MODE_HPP
#define TAO_PEGTL_APPLY_MODE_HPP



namespace TAO_PEGTL_NAMESPACE
{
   enum class apply_mode : bool
   {
      action = true,
      nothing = false
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 10 "tao/pegtl/parse.hpp"

#line 1 "tao/pegtl/normal.hpp"
       
#line 1 "tao/pegtl/normal.hpp"



#ifndef TAO_PEGTL_NORMAL_HPP
#define TAO_PEGTL_NORMAL_HPP

#include <string>
#include <type_traits>
#include <utility>



#line 1 "tao/pegtl/demangle.hpp"
       
#line 1 "tao/pegtl/demangle.hpp"



#ifndef TAO_PEGTL_DEMANGLE_HPP
#define TAO_PEGTL_DEMANGLE_HPP

#include <ciso646>
#include <string_view>

namespace tao
{
#if defined( __clang__ )

#if defined( _LIBCPP_VERSION )

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      constexpr std::string_view sv = __PRETTY_FUNCTION__;
      constexpr auto begin = sv.find( '=' );
      static_assert( begin != std::string_view::npos );
      return sv.substr( begin + 2, sv.size() - begin - 3 );
   }

#else

   // When using libstdc++ with clang, std::string_view::find is not constexpr :(
   template< char C >
   constexpr const char* find( const char* p, std::size_t n ) noexcept
   {
      while( n ) {
         if( *p == C ) {
            return p;
         }
         ++p;
         --n;
      }
      return nullptr;
   }

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      constexpr std::string_view sv = __PRETTY_FUNCTION__;
      constexpr auto begin = find< '=' >( sv.data(), sv.size() );
      static_assert( begin != nullptr );
      return { begin + 2, sv.data() + sv.size() - begin - 3 };
   }

#endif

#elif defined( __GNUC__ )

#if( __GNUC__ == 7 )

   // GCC 7 wrongly sometimes disallows __PRETTY_FUNCTION__ in constexpr functions,
   // therefore we drop the 'constexpr' and hope for the best.
   template< typename T >
   [[nodiscard]] std::string_view demangle() noexcept
   {
      const std::string_view sv = __PRETTY_FUNCTION__;
      const auto begin = sv.find( '=' );
      const auto tmp = sv.substr( begin + 2 );
      const auto end = tmp.rfind( ';' );
      return tmp.substr( 0, end );
   }

#elif( __GNUC__ == 9 ) && ( __GNUC_MINOR__ < 3 )

   // GCC 9.1 and 9.2 have a bug that leads to truncated __PRETTY_FUNCTION__ names,
   // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91155
   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      // fallback: requires RTTI, no demangling
      return typeid( T ).name();
   }

#else

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      constexpr std::string_view sv = __PRETTY_FUNCTION__;
      constexpr auto begin = sv.find( '=' );
      static_assert( begin != std::string_view::npos );
      constexpr auto tmp = sv.substr( begin + 2 );
      constexpr auto end = tmp.rfind( ';' );
      static_assert( end != std::string_view::npos );
      return tmp.substr( 0, end );
   }

#endif

#elif defined( _MSC_VER )

#if( _MSC_VER < 1920 )

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      const std::string_view sv = __FUNCSIG__;
      const auto begin = sv.find( "demangle<" );
      const auto tmp = sv.substr( begin + 9 );
      const auto end = tmp.rfind( '>' );
      return tmp.substr( 0, end );
   }

#else

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      constexpr std::string_view sv = __FUNCSIG__;
      constexpr auto begin = sv.find( "demangle<" );
      static_assert( begin != std::string_view::npos );
      constexpr auto tmp = sv.substr( begin + 9 );
      constexpr auto end = tmp.rfind( '>' );
      static_assert( end != std::string_view::npos );
      return tmp.substr( 0, end );
   }

#endif

#else

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      // fallback: requires RTTI, no demangling
      return typeid( T ).name();
   }

#endif

} // namespace tao

#endif
#line 14 "tao/pegtl/normal.hpp"
#line 1 "tao/pegtl/match.hpp"
       
#line 1 "tao/pegtl/match.hpp"



#ifndef TAO_PEGTL_MATCH_HPP
#define TAO_PEGTL_MATCH_HPP

#include <type_traits>



#line 1 "tao/pegtl/nothing.hpp"
       
#line 1 "tao/pegtl/nothing.hpp"



#ifndef TAO_PEGTL_NOTHING_HPP
#define TAO_PEGTL_NOTHING_HPP



namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct nothing
   {};

   using maybe_nothing = nothing< void >;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 12 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/require_apply.hpp"
       
#line 1 "tao/pegtl/require_apply.hpp"



#ifndef TAO_PEGTL_REQUIRE_APPLY_HPP
#define TAO_PEGTL_REQUIRE_APPLY_HPP



namespace TAO_PEGTL_NAMESPACE
{
   struct require_apply
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 13 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/require_apply0.hpp"
       
#line 1 "tao/pegtl/require_apply0.hpp"



#ifndef TAO_PEGTL_REQUIRE_APPLY0_HPP
#define TAO_PEGTL_REQUIRE_APPLY0_HPP



namespace TAO_PEGTL_NAMESPACE
{
   struct require_apply0
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 14 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/rewind_mode.hpp"
       
#line 1 "tao/pegtl/rewind_mode.hpp"



#ifndef TAO_PEGTL_REWIND_MODE_HPP
#define TAO_PEGTL_REWIND_MODE_HPP



namespace TAO_PEGTL_NAMESPACE
{
   enum class rewind_mode : char
   {
      active,
      required,
      dontcare
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 15 "tao/pegtl/match.hpp"

#line 1 "tao/pegtl/internal/has_apply.hpp"
       
#line 1 "tao/pegtl/internal/has_apply.hpp"



#ifndef TAO_PEGTL_INTERNAL_HAS_APPLY_HPP
#define TAO_PEGTL_INTERNAL_HAS_APPLY_HPP

#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename, typename, template< typename... > class, typename... >
   inline constexpr bool has_apply = false;

   template< typename C, template< typename... > class Action, typename... S >
   inline constexpr bool has_apply< C, decltype( C::template apply< Action >( std::declval< S >()... ) ), Action, S... > = true;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 17 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/internal/has_apply0.hpp"
       
#line 1 "tao/pegtl/internal/has_apply0.hpp"



#ifndef TAO_PEGTL_INTERNAL_HAS_APPLY0_HPP
#define TAO_PEGTL_INTERNAL_HAS_APPLY0_HPP

#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename, typename, template< typename... > class, typename... >
   inline constexpr bool has_apply0 = false;

   template< typename C, template< typename... > class Action, typename... S >
   inline constexpr bool has_apply0< C, decltype( C::template apply0< Action >( std::declval< S >()... ) ), Action, S... > = true;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 18 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/internal/has_unwind.hpp"
       
#line 1 "tao/pegtl/internal/has_unwind.hpp"



#ifndef TAO_PEGTL_INTERNAL_HAS_UNWIND_HPP
#define TAO_PEGTL_INTERNAL_HAS_UNWIND_HPP

#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename, typename... >
   inline constexpr bool has_unwind = false;

   template< typename C, typename... S >
   inline constexpr bool has_unwind< C, decltype( C::unwind( std::declval< S >()... ) ), S... > = true;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 19 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/internal/marker.hpp"
       
#line 1 "tao/pegtl/internal/marker.hpp"



#ifndef TAO_PEGTL_INTERNAL_MARKER_HPP
#define TAO_PEGTL_INTERNAL_MARKER_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Iterator, rewind_mode M >
   class marker
   {
   public:
      static constexpr rewind_mode next_rewind_mode = M;

      explicit marker( const Iterator& /*unused*/ ) noexcept
      {}

      marker( const marker& ) = delete;
      marker( marker&& ) = delete;

      ~marker() = default;

      void operator=( const marker& ) = delete;
      void operator=( marker&& ) = delete;

      [[nodiscard]] bool operator()( const bool result ) const noexcept
      {
         return result;
      }
   };

   template< typename Iterator >
   class marker< Iterator, rewind_mode::required >
   {
   public:
      static constexpr rewind_mode next_rewind_mode = rewind_mode::active;

      explicit marker( Iterator& i ) noexcept
         : m_saved( i ),
           m_input( &i )
      {}

      marker( const marker& ) = delete;
      marker( marker&& ) = delete;

      ~marker()
      {
         if( m_input != nullptr ) {
            ( *m_input ) = m_saved;
         }
      }

      void operator=( const marker& ) = delete;
      void operator=( marker&& ) = delete;

      [[nodiscard]] bool operator()( const bool result ) noexcept
      {
         if( result ) {
            m_input = nullptr;
            return true;
         }
         return false;
      }

      [[nodiscard]] const Iterator& iterator() const noexcept
      {
         return m_saved;
      }

   private:
      const Iterator m_saved;
      Iterator* m_input;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 20 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/internal/missing_apply.hpp"
       
#line 1 "tao/pegtl/internal/missing_apply.hpp"



#ifndef TAO_PEGTL_INTERNAL_MISSING_APPLY_HPP
#define TAO_PEGTL_INTERNAL_MISSING_APPLY_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Control,
             template< typename... >
             class Action,
             typename ParseInput,
             typename... States >
   void missing_apply( ParseInput& in, States&&... st )
   {
      // This function only exists for better error messages, which means that it is only called when we know that it won't compile.
      // LCOV_EXCL_START
      auto m = in.template mark< rewind_mode::required >();
      (void)Control::template apply< Action >( m.iterator(), in, st... );
      // LCOV_EXCL_STOP
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 21 "tao/pegtl/match.hpp"
#line 1 "tao/pegtl/internal/missing_apply0.hpp"
       
#line 1 "tao/pegtl/internal/missing_apply0.hpp"



#ifndef TAO_PEGTL_INTERNAL_MISSING_APPLY0_HPP
#define TAO_PEGTL_INTERNAL_MISSING_APPLY0_HPP



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Control,
             template< typename... >
             class Action,
             typename ParseInput,
             typename... States >
   void missing_apply0( ParseInput& in, States&&... st )
   {
      // This function only exists for better error messages, which means that it is only called when we know that it won't compile.
      // LCOV_EXCL_START
      (void)Control::template apply0< Action >( in, st... );
      // LCOV_EXCL_STOP
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 22 "tao/pegtl/match.hpp"

#if defined( _MSC_VER )
#pragma warning( push )
#pragma warning( disable : 4702 )
#endif

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static auto match_no_control( ParseInput& in, States&&... st )
         -> decltype( Rule::template match< A, M, Action, Control >( in, st... ) )
      {
         return Rule::template match< A, M, Action, Control >( in, st... );
      }

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static auto match_no_control( ParseInput& in, States&&... /*unused*/ )
         -> decltype( Rule::match( in ) )
      {
         return Rule::match( in );
      }

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] auto match_control_unwind( ParseInput& in, States&&... st )
      {
         if constexpr( has_unwind< Control< Rule >, void, const ParseInput&, States... > ) {
            try {
               return match_no_control< Rule, A, M, Action, Control >( in, st... );
            }
            catch( ... ) {
               Control< Rule >::unwind( static_cast< const ParseInput& >( in ), st... );
               throw;
            }
         }
         else {
            return match_no_control< Rule, A, M, Action, Control >( in, st... );
         }
      }

   } // namespace internal

   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... >
             class Action,
             template< typename... >
             class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] auto match( ParseInput& in, States&&... st )
   {
      if constexpr( !Control< Rule >::enable ) {
         return internal::match_no_control< Rule, A, M, Action, Control >( in, st... );
      }
      else {
         constexpr bool enable_action = ( A == apply_mode::action );

         using iterator_t = typename ParseInput::iterator_t;
         constexpr bool has_apply_void = enable_action && internal::has_apply< Control< Rule >, void, Action, const iterator_t&, const ParseInput&, States... >;
         constexpr bool has_apply_bool = enable_action && internal::has_apply< Control< Rule >, bool, Action, const iterator_t&, const ParseInput&, States... >;
         constexpr bool has_apply = has_apply_void || has_apply_bool;

         constexpr bool has_apply0_void = enable_action && internal::has_apply0< Control< Rule >, void, Action, const ParseInput&, States... >;
         constexpr bool has_apply0_bool = enable_action && internal::has_apply0< Control< Rule >, bool, Action, const ParseInput&, States... >;
         constexpr bool has_apply0 = has_apply0_void || has_apply0_bool;

         static_assert( !( has_apply && has_apply0 ), "both apply() and apply0() defined" );

         constexpr bool is_nothing = std::is_base_of_v< nothing< Rule >, Action< Rule > >;
         static_assert( !( has_apply && is_nothing ), "unexpected apply() defined" );
         static_assert( !( has_apply0 && is_nothing ), "unexpected apply0() defined" );

         if constexpr( !has_apply && std::is_base_of_v< require_apply, Action< Rule > > ) {
            internal::missing_apply< Control< Rule >, Action >( in, st... );
         }

         if constexpr( !has_apply0 && std::is_base_of_v< require_apply0, Action< Rule > > ) {
            internal::missing_apply0< Control< Rule >, Action >( in, st... );
         }

         constexpr bool validate_nothing = std::is_base_of_v< maybe_nothing, Action< void > >;
         constexpr bool is_maybe_nothing = std::is_base_of_v< maybe_nothing, Action< Rule > >;
         static_assert( !enable_action || !validate_nothing || is_nothing || is_maybe_nothing || has_apply || has_apply0, "either apply() or apply0() must be defined" );

         constexpr bool use_marker = has_apply || has_apply0_bool;

         auto m = in.template mark< ( use_marker ? rewind_mode::required : rewind_mode::dontcare ) >();
         Control< Rule >::start( static_cast< const ParseInput& >( in ), st... );
         auto result = internal::match_control_unwind< Rule, A, ( use_marker ? rewind_mode::active : M ), Action, Control >( in, st... );
         if( result ) {
            if constexpr( has_apply_void ) {
               Control< Rule >::template apply< Action >( m.iterator(), static_cast< const ParseInput& >( in ), st... );
            }
            else if constexpr( has_apply_bool ) {
               result = Control< Rule >::template apply< Action >( m.iterator(), static_cast< const ParseInput& >( in ), st... );
            }
            else if constexpr( has_apply0_void ) {
               Control< Rule >::template apply0< Action >( static_cast< const ParseInput& >( in ), st... );
            }
            else if constexpr( has_apply0_bool ) {
               result = Control< Rule >::template apply0< Action >( static_cast< const ParseInput& >( in ), st... );
            }
         }
         if( result ) {
            Control< Rule >::success( static_cast< const ParseInput& >( in ), st... );
         }
         else {
            Control< Rule >::failure( static_cast< const ParseInput& >( in ), st... );
         }
         (void)m( result );
         return result;
      }
   }

} // namespace TAO_PEGTL_NAMESPACE

#if defined( _MSC_VER )
#pragma warning( pop )
#endif

#endif
#line 15 "tao/pegtl/normal.hpp"
#line 1 "tao/pegtl/parse_error.hpp"
       
#line 1 "tao/pegtl/parse_error.hpp"



#ifndef TAO_PEGTL_PARSE_ERROR_HPP
#define TAO_PEGTL_PARSE_ERROR_HPP

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


#line 1 "tao/pegtl/position.hpp"
       
#line 1 "tao/pegtl/position.hpp"



#ifndef TAO_PEGTL_POSITION_HPP
#define TAO_PEGTL_POSITION_HPP

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>



#line 1 "tao/pegtl/internal/iterator.hpp"
       
#line 1 "tao/pegtl/internal/iterator.hpp"



#ifndef TAO_PEGTL_INTERNAL_ITERATOR_HPP
#define TAO_PEGTL_INTERNAL_ITERATOR_HPP

#include <cassert>
#include <cstdlib>



namespace TAO_PEGTL_NAMESPACE::internal
{
   struct iterator
   {
      iterator() = default;

      explicit iterator( const char* in_data ) noexcept
         : data( in_data )
      {}

      iterator( const char* in_data, const std::size_t in_byte, const std::size_t in_line, const std::size_t in_column ) noexcept
         : data( in_data ),
           byte( in_byte ),
           line( in_line ),
           column( in_column )
      {
         assert( in_line != 0 );
         assert( in_column != 0 );
      }

      iterator( const iterator& ) = default;
      iterator( iterator&& ) = default;

      ~iterator() = default;

      iterator& operator=( const iterator& ) = default;
      iterator& operator=( iterator&& ) = default;

      const char* data = nullptr;

      std::size_t byte = 0;
      std::size_t line = 1;
      std::size_t column = 1;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 16 "tao/pegtl/position.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   struct position
   {
      position() = delete;

      position( position&& p ) noexcept
         : byte( p.byte ),
           line( p.line ),
           column( p.column ),
           source( std::move( p.source ) )
      {}

      position( const position& ) = default;

      position& operator=( position&& p ) noexcept
      {
         byte = p.byte;
         line = p.line;
         column = p.column;
         source = std::move( p.source );
         return *this;
      }

      position& operator=( const position& ) = default;

      template< typename T >
      position( const internal::iterator& in_iter, T&& in_source )
         : byte( in_iter.byte ),
           line( in_iter.line ),
           column( in_iter.column ),
           source( std::forward< T >( in_source ) )
      {}

      ~position() = default;

      std::size_t byte;
      std::size_t line;
      std::size_t column;
      std::string source;
   };

   inline bool operator==( const position& lhs, const position& rhs ) noexcept
   {
      return ( lhs.byte == rhs.byte ) && ( lhs.source == rhs.source );
   }

   inline bool operator!=( const position& lhs, const position& rhs ) noexcept
   {
      return !( lhs == rhs );
   }

   inline std::ostream& operator<<( std::ostream& os, const position& p )
   {
      return os << p.source << ':' << p.line << ':' << p.column;
   }

   [[nodiscard]] inline std::string to_string( const position& p )
   {
      std::ostringstream o;
      o << p;
      return o.str();
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 17 "tao/pegtl/parse_error.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      class parse_error
      {
      private:
         std::string m_msg;
         std::size_t m_prefix = 0;
         std::vector< position > m_positions;

      public:
         explicit parse_error( const char* msg )
            : m_msg( msg )
         {}

         [[nodiscard]] const char* what() const noexcept
         {
            return m_msg.c_str();
         }

         [[nodiscard]] std::string_view message() const noexcept
         {
            return { m_msg.data() + m_prefix, m_msg.size() - m_prefix };
         }

         [[nodiscard]] const std::vector< position >& positions() const noexcept
         {
            return m_positions;
         }

         void add_position( position&& p )
         {
            const auto prefix = to_string( p );
            m_msg = prefix + ": " + m_msg;
            m_prefix += prefix.size() + 2;
            m_positions.emplace_back( std::move( p ) );
         }
      };

   } // namespace internal

   class parse_error
      : public std::runtime_error
   {
   private:
      std::shared_ptr< internal::parse_error > m_impl;

   public:
      parse_error( const char* msg, position p )
         : std::runtime_error( msg ),
           m_impl( std::make_shared< internal::parse_error >( msg ) )
      {
         m_impl->add_position( std::move( p ) );
      }

      parse_error( const std::string& msg, position p )
         : parse_error( msg.c_str(), std::move( p ) )
      {}

      template< typename ParseInput >
      parse_error( const char* msg, const ParseInput& in )
         : parse_error( msg, in.position() )
      {}

      template< typename ParseInput >
      parse_error( const std::string& msg, const ParseInput& in )
         : parse_error( msg.c_str(), in.position() )
      {}

      [[nodiscard]] const char* what() const noexcept override
      {
         return m_impl->what();
      }

      [[nodiscard]] std::string_view message() const noexcept
      {
         return m_impl->message();
      }

      [[nodiscard]] const std::vector< position >& positions() const noexcept
      {
         return m_impl->positions();
      }

      void add_position( position&& p )
      {
         if( m_impl.use_count() > 1 ) {
            m_impl = std::make_shared< internal::parse_error >( *m_impl );
         }
         m_impl->add_position( std::move( p ) );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 16 "tao/pegtl/normal.hpp"


#line 1 "tao/pegtl/internal/enable_control.hpp"
       
#line 1 "tao/pegtl/internal/enable_control.hpp"



#ifndef TAO_PEGTL_INTERNAL_ENABLE_CONTROL_HPP
#define TAO_PEGTL_INTERNAL_ENABLE_CONTROL_HPP

#include <type_traits>



namespace TAO_PEGTL_NAMESPACE::internal
{
   // This class is a simple tagging mechanism.
   // By default, enable_control< Rule > is  'true'.
   // Each internal (!) rule that should be hidden
   // from the control and action class' callbacks
   // simply specializes enable_control<> to return
   // 'true' for the above expression.

   template< typename Rule >
   inline constexpr bool enable_control = true;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 19 "tao/pegtl/normal.hpp"
#line 1 "tao/pegtl/internal/has_match.hpp"
       
#line 1 "tao/pegtl/internal/has_match.hpp"



#ifndef TAO_PEGTL_INTERNAL_HAS_MATCH_HPP
#define TAO_PEGTL_INTERNAL_HAS_MATCH_HPP

#include <utility>





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename,
             typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... >
             class Action,
             template< typename... >
             class Control,
             typename ParseInput,
             typename... States >
   inline constexpr bool has_match = false;

   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... >
             class Action,
             template< typename... >
             class Control,
             typename ParseInput,
             typename... States >
   inline constexpr bool has_match< decltype( (void)Action< Rule >::template match< Rule, A, M, Action, Control >( std::declval< ParseInput& >(), std::declval< States&& >()... ), bool() ), Rule, A, M, Action, Control, ParseInput, States... > = true;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 20 "tao/pegtl/normal.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct normal
   {
      static constexpr bool enable = internal::enable_control< Rule >;

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... /*unused*/ )
      {
         throw parse_error( "parse error matching " + std::string( demangle< Rule >() ), in );
      }

      template< template< typename... > class Action,
                typename Iterator,
                typename ParseInput,
                typename... States >
      static auto apply( const Iterator& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( Action< Rule >::apply( std::declval< const typename ParseInput::action_t& >(), st... ) ) )
         -> decltype( Action< Rule >::apply( std::declval< const typename ParseInput::action_t& >(), st... ) )
      {
         const typename ParseInput::action_t action_input( begin, in );
         return Action< Rule >::apply( action_input, st... );
      }

      template< template< typename... > class Action,
                typename ParseInput,
                typename... States >
      static auto apply0( const ParseInput& /*unused*/, States&&... st ) noexcept( noexcept( Action< Rule >::apply0( st... ) ) )
         -> decltype( Action< Rule >::apply0( st... ) )
      {
         return Action< Rule >::apply0( st... );
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( internal::has_match< bool, Rule, A, M, Action, Control, ParseInput, States... > ) {
            return Action< Rule >::template match< Rule, A, M, Action, Control >( in, st... );
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 12 "tao/pegtl/parse.hpp"




namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             apply_mode A = apply_mode::action,
             rewind_mode M = rewind_mode::required,
             typename ParseInput,
             typename... States >
   auto parse( ParseInput&& in, States&&... st )
   {
      return Control< Rule >::template match< A, M, Action, Control >( in, st... );
   }

   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             apply_mode A = apply_mode::action,
             rewind_mode M = rewind_mode::required,
             typename OuterInput,
             typename ParseInput,
             typename... States >
   auto parse_nested( const OuterInput& oi, ParseInput&& in, States&&... st )
   {
      try {
         return parse< Rule, Action, Control, A, M >( in, st... );
      }
      catch( parse_error& e ) {
         e.add_position( oi.position() );
         throw;
      }
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 9 "tao/pegtl.hpp"
#line 1 "tao/pegtl/version.hpp"
       
#line 1 "tao/pegtl/version.hpp"



#ifndef TAO_PEGTL_VERSION_HPP
#define TAO_PEGTL_VERSION_HPP

#define TAO_PEGTL_VERSION "3.0.0"

#define TAO_PEGTL_VERSION_MAJOR 3
#define TAO_PEGTL_VERSION_MINOR 0
#define TAO_PEGTL_VERSION_PATCH 0

#endif
#line 10 "tao/pegtl.hpp"

#line 1 "tao/pegtl/ascii.hpp"
       
#line 1 "tao/pegtl/ascii.hpp"



#ifndef TAO_PEGTL_ASCII_HPP
#define TAO_PEGTL_ASCII_HPP



#line 1 "tao/pegtl/internal/dependent_false.hpp"
       
#line 1 "tao/pegtl/internal/dependent_false.hpp"



#ifndef TAO_PEGTL_INTERNAL_DEPENDENT_FALSE_HPP
#define TAO_PEGTL_INTERNAL_DEPENDENT_FALSE_HPP



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... >
   inline constexpr bool dependent_false = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/ascii.hpp"
#line 1 "tao/pegtl/internal/result_on_found.hpp"
       
#line 1 "tao/pegtl/internal/result_on_found.hpp"



#ifndef TAO_PEGTL_INTERNAL_RESULT_ON_FOUND_HPP
#define TAO_PEGTL_INTERNAL_RESULT_ON_FOUND_HPP



namespace TAO_PEGTL_NAMESPACE::internal
{
   enum class result_on_found : bool
   {
      success = true,
      failure = false
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/ascii.hpp"
#line 1 "tao/pegtl/internal/rules.hpp"
       
#line 1 "tao/pegtl/internal/rules.hpp"



#ifndef TAO_PEGTL_INTERNAL_RULES_HPP
#define TAO_PEGTL_INTERNAL_RULES_HPP

#line 1 "tao/pegtl/internal/action.hpp"
       
#line 1 "tao/pegtl/internal/action.hpp"



#ifndef TAO_PEGTL_INTERNAL_ACTION_HPP
#define TAO_PEGTL_INTERNAL_ACTION_HPP




#line 1 "tao/pegtl/internal/seq.hpp"
       
#line 1 "tao/pegtl/internal/seq.hpp"



#ifndef TAO_PEGTL_INTERNAL_SEQ_HPP
#define TAO_PEGTL_INTERNAL_SEQ_HPP




#line 1 "tao/pegtl/internal/success.hpp"
       
#line 1 "tao/pegtl/internal/success.hpp"



#ifndef TAO_PEGTL_INTERNAL_SUCCESS_HPP
#define TAO_PEGTL_INTERNAL_SUCCESS_HPP





#line 1 "tao/pegtl/internal/../type_list.hpp"
       
#line 1 "tao/pegtl/internal/../type_list.hpp"



#ifndef TAO_PEGTL_TYPE_LIST_HPP
#define TAO_PEGTL_TYPE_LIST_HPP

#include <cstddef>



namespace TAO_PEGTL_NAMESPACE
{
   template< typename... Ts >
   struct type_list
   {
      static constexpr std::size_t size = sizeof...( Ts );
   };

   using empty_list = type_list<>;

   template< typename... >
   struct type_list_concat;

   template<>
   struct type_list_concat<>
   {
      using type = empty_list;
   };

   template< typename... Ts >
   struct type_list_concat< type_list< Ts... > >
   {
      using type = type_list< Ts... >;
   };

   template< typename... T0s, typename... T1s, typename... Ts >
   struct type_list_concat< type_list< T0s... >, type_list< T1s... >, Ts... >
      : type_list_concat< type_list< T0s..., T1s... >, Ts... >
   {};

   template< typename... Ts >
   using type_list_concat_t = typename type_list_concat< Ts... >::type;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 12 "tao/pegtl/internal/success.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   struct success
   {
      using rule_t = success;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& /*unused*/ ) noexcept
      {
         return true;
      }
   };

   template<>
   inline constexpr bool enable_control< success > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/seq.hpp"





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct seq;

   template<>
   struct seq<>
      : success
   {};

   template< typename... Rules >
   struct seq
   {
      using rule_t = seq;
      using subs_t = type_list< Rules... >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( sizeof...( Rules ) == 1 ) {
            return Control< Rules... >::template match< A, M, Action, Control >( in, st... );
         }
         else {
            auto m = in.template mark< M >();
            using m_t = decltype( m );
            return m( ( Control< Rules >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) && ... ) );
         }
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< seq< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/action.hpp"






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< template< typename... > class Action, typename... Rules >
   struct action
      : action< Action, seq< Rules... > >
   {};

   template< template< typename... > class Action >
   struct action< Action >
      : success
   {};

   template< template< typename... > class Action, typename Rule >
   struct action< Action, Rule >
   {
      using rule_t = action;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return Control< Rule >::template match< A, M, Action, Control >( in, st... );
      }
   };

   template< template< typename... > class Action, typename... Rules >
   inline constexpr bool enable_control< action< Action, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 8 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/any.hpp"
       
#line 1 "tao/pegtl/internal/any.hpp"



#ifndef TAO_PEGTL_INTERNAL_ANY_HPP
#define TAO_PEGTL_INTERNAL_ANY_HPP




#line 1 "tao/pegtl/internal/peek_char.hpp"
       
#line 1 "tao/pegtl/internal/peek_char.hpp"



#ifndef TAO_PEGTL_INTERNAL_PEEK_CHAR_HPP
#define TAO_PEGTL_INTERNAL_PEEK_CHAR_HPP

#include <cstddef>



#line 1 "tao/pegtl/internal/input_pair.hpp"
       
#line 1 "tao/pegtl/internal/input_pair.hpp"



#ifndef TAO_PEGTL_INTERNAL_INPUT_PAIR_HPP
#define TAO_PEGTL_INTERNAL_INPUT_PAIR_HPP

#include <cstdint>



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Data >
   struct input_pair
   {
      Data data;
      std::uint8_t size;

      using data_t = Data;

      explicit operator bool() const noexcept
      {
         return size > 0;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/peek_char.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   struct peek_char
   {
      using data_t = char;
      using pair_t = input_pair< char >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( in.empty() ) {
            return { 0, 0 };
         }
         return { in.peek_char(), 1 };
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/any.hpp"



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Peek >
   struct any;

   template<>
   struct any< peek_char >
   {
      using rule_t = any;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( !in.empty() ) {
            in.bump();
            return true;
         }
         return false;
      }
   };

   template< typename Peek >
   struct any
   {
      using rule_t = any;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( Peek::peek( in ) ) )
      {
         if( const auto t = Peek::peek( in ) ) {
            in.bump( t.size );
            return true;
         }
         return false;
      }
   };

   template< typename Peek >
   inline constexpr bool enable_control< any< Peek > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 9 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/apply.hpp"
       
#line 1 "tao/pegtl/internal/apply.hpp"



#ifndef TAO_PEGTL_INTERNAL_APPLY_HPP
#define TAO_PEGTL_INTERNAL_APPLY_HPP



#line 1 "tao/pegtl/internal/apply_single.hpp"
       
#line 1 "tao/pegtl/internal/apply_single.hpp"



#ifndef TAO_PEGTL_INTERNAL_APPLY_SINGLE_HPP
#define TAO_PEGTL_INTERNAL_APPLY_SINGLE_HPP



#include <type_traits>

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Action >
   struct apply_single
   {
      template< typename ActionInput, typename... States >
      [[nodiscard]] static auto match( const ActionInput& in, States&&... st ) noexcept( noexcept( Action::apply( in, st... ) ) )
         -> std::enable_if_t< std::is_same_v< decltype( Action::apply( in, st... ) ), void >, bool >
      {
         Action::apply( in, st... );
         return true;
      }

      template< typename ActionInput, typename... States >
      [[nodiscard]] static auto match( const ActionInput& in, States&&... st ) noexcept( noexcept( Action::apply( in, st... ) ) )
         -> std::enable_if_t< std::is_same_v< decltype( Action::apply( in, st... ) ), bool >, bool >
      {
         return Action::apply( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/internal/apply.hpp"






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Actions >
   struct apply
   {
      using rule_t = apply;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( [[maybe_unused]] ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( ( A == apply_mode::action ) && ( sizeof...( Actions ) > 0 ) ) {
            using action_t = typename ParseInput::action_t;
            const action_t i2( in.iterator(), in ); // No data -- range is from begin to begin.
            return ( apply_single< Actions >::match( i2, st... ) && ... );
         }
         else {
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
            return true;
         }
      }
   };

   template< typename... Actions >
   inline constexpr bool enable_control< apply< Actions... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/apply0.hpp"
       
#line 1 "tao/pegtl/internal/apply0.hpp"



#ifndef TAO_PEGTL_INTERNAL_APPLY0_HPP
#define TAO_PEGTL_INTERNAL_APPLY0_HPP



#line 1 "tao/pegtl/internal/apply0_single.hpp"
       
#line 1 "tao/pegtl/internal/apply0_single.hpp"



#ifndef TAO_PEGTL_INTERNAL_APPLY0_SINGLE_HPP
#define TAO_PEGTL_INTERNAL_APPLY0_SINGLE_HPP



#include <type_traits>

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Action >
   struct apply0_single
   {
      template< typename... States >
      [[nodiscard]] static auto match( States&&... st ) noexcept( noexcept( Action::apply0( st... ) ) )
         -> std::enable_if_t< std::is_same_v< decltype( Action::apply0( st... ) ), void >, bool >
      {
         Action::apply0( st... );
         return true;
      }

      template< typename... States >
      [[nodiscard]] static auto match( States&&... st ) noexcept( noexcept( Action::apply0( st... ) ) )
         -> std::enable_if_t< std::is_same_v< decltype( Action::apply0( st... ) ), bool >, bool >
      {
         return Action::apply0( st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/internal/apply0.hpp"






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Actions >
   struct apply0
   {
      using rule_t = apply0;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& /*unused*/, [[maybe_unused]] States&&... st )
      {
         if constexpr( A == apply_mode::action ) {
            return ( apply0_single< Actions >::match( st... ) && ... );
         }
         else {
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
            return true;
         }
      }
   };

   template< typename... Actions >
   inline constexpr bool enable_control< apply0< Actions... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/at.hpp"
       
#line 1 "tao/pegtl/internal/at.hpp"



#ifndef TAO_PEGTL_INTERNAL_AT_HPP
#define TAO_PEGTL_INTERNAL_AT_HPP
#line 17 "tao/pegtl/internal/at.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct at
      : at< seq< Rules... > >
   {};

   template<>
   struct at<>
      : success
   {};

   template< typename Rule >
   struct at< Rule >
   {
      using rule_t = at;
      using subs_t = type_list< Rule >;

      template< apply_mode,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         const auto m = in.template mark< rewind_mode::required >();
         return Control< Rule >::template match< apply_mode::nothing, rewind_mode::active, Action, Control >( in, st... );
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< at< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/bof.hpp"
       
#line 1 "tao/pegtl/internal/bof.hpp"



#ifndef TAO_PEGTL_INTERNAL_BOF_HPP
#define TAO_PEGTL_INTERNAL_BOF_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct bof
   {
      using rule_t = bof;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept
      {
         return in.byte() == 0;
      }
   };

   template<>
   inline constexpr bool enable_control< bof > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/bol.hpp"
       
#line 1 "tao/pegtl/internal/bol.hpp"



#ifndef TAO_PEGTL_INTERNAL_BOL_HPP
#define TAO_PEGTL_INTERNAL_BOL_HPP






namespace TAO_PEGTL_NAMESPACE::internal
{
   struct bol
   {
      using rule_t = bol;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept
      {
         return in.column() == 1;
      }
   };

   template<>
   inline constexpr bool enable_control< bol > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/bytes.hpp"
       
#line 1 "tao/pegtl/internal/bytes.hpp"



#ifndef TAO_PEGTL_INTERNAL_BYTES_HPP
#define TAO_PEGTL_INTERNAL_BYTES_HPP
#line 14 "tao/pegtl/internal/bytes.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Cnt >
   struct bytes
   {
      using rule_t = bytes;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.size( 0 ) ) )
      {
         if( in.size( Cnt ) >= Cnt ) {
            in.bump( Cnt );
            return true;
         }
         return false;
      }
   };

   template<>
   struct bytes< 0 >
      : success
   {};

   template< unsigned Cnt >
   inline constexpr bool enable_control< bytes< Cnt > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 15 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/control.hpp"
       
#line 1 "tao/pegtl/internal/control.hpp"



#ifndef TAO_PEGTL_INTERNAL_CONTROL_HPP
#define TAO_PEGTL_INTERNAL_CONTROL_HPP
#line 17 "tao/pegtl/internal/control.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< template< typename... > class Control, typename... Rules >
   struct control
      : control< Control, seq< Rules... > >
   {};

   template< template< typename... > class Control >
   struct control< Control >
      : success
   {};

   template< template< typename... > class Control, typename Rule >
   struct control< Control, Rule >
   {
      using rule_t = control;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return Control< Rule >::template match< A, M, Action, Control >( in, st... );
      }
   };

   template< template< typename... > class Control, typename... Rules >
   inline constexpr bool enable_control< control< Control, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 16 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/disable.hpp"
       
#line 1 "tao/pegtl/internal/disable.hpp"



#ifndef TAO_PEGTL_INTERNAL_DISABLE_HPP
#define TAO_PEGTL_INTERNAL_DISABLE_HPP
#line 17 "tao/pegtl/internal/disable.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct disable
      : disable< seq< Rules... > >
   {};

   template<>
   struct disable<>
      : success
   {};

   template< typename Rule >
   struct disable< Rule >
   {
      using rule_t = disable;
      using subs_t = type_list< Rule >;

      template< apply_mode,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return Control< Rule >::template match< apply_mode::nothing, M, Action, Control >( in, st... );
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< disable< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 17 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/discard.hpp"
       
#line 1 "tao/pegtl/internal/discard.hpp"



#ifndef TAO_PEGTL_INTERNAL_DISCARD_HPP
#define TAO_PEGTL_INTERNAL_DISCARD_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct discard
   {
      using rule_t = discard;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept
      {
         static_assert( noexcept( in.discard() ) );
         in.discard();
         return true;
      }
   };

   template<>
   inline constexpr bool enable_control< discard > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 18 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/enable.hpp"
       
#line 1 "tao/pegtl/internal/enable.hpp"



#ifndef TAO_PEGTL_INTERNAL_ENABLE_HPP
#define TAO_PEGTL_INTERNAL_ENABLE_HPP
#line 17 "tao/pegtl/internal/enable.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct enable
      : enable< seq< Rules... > >
   {};

   template<>
   struct enable<>
      : success
   {};

   template< typename Rule >
   struct enable< Rule >
   {
      using rule_t = enable;
      using subs_t = type_list< Rule >;

      template< apply_mode,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return Control< Rule >::template match< apply_mode::action, M, Action, Control >( in, st... );
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< enable< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 19 "tao/pegtl/internal/rules.hpp"

#line 1 "tao/pegtl/internal/eof.hpp"
       
#line 1 "tao/pegtl/internal/eof.hpp"



#ifndef TAO_PEGTL_INTERNAL_EOF_HPP
#define TAO_PEGTL_INTERNAL_EOF_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct eof
   {
      using rule_t = eof;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         return in.empty();
      }
   };

   template<>
   inline constexpr bool enable_control< eof > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 21 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/eol.hpp"
       
#line 1 "tao/pegtl/internal/eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_EOL_HPP
#define TAO_PEGTL_INTERNAL_EOL_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct eol
   {
      using rule_t = eol;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( ParseInput::eol_t::match( in ) ) )
      {
         return ParseInput::eol_t::match( in ).first;
      }
   };

   template<>
   inline constexpr bool enable_control< eol > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 22 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/eolf.hpp"
       
#line 1 "tao/pegtl/internal/eolf.hpp"



#ifndef TAO_PEGTL_INTERNAL_EOLF_HPP
#define TAO_PEGTL_INTERNAL_EOLF_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct eolf
   {
      using rule_t = eolf;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( ParseInput::eol_t::match( in ) ) )
      {
         const auto p = ParseInput::eol_t::match( in );
         return p.first || ( !p.second );
      }
   };

   template<>
   inline constexpr bool enable_control< eolf > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 23 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/failure.hpp"
       
#line 1 "tao/pegtl/internal/failure.hpp"



#ifndef TAO_PEGTL_INTERNAL_FAILURE_HPP
#define TAO_PEGTL_INTERNAL_FAILURE_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   struct failure
   {
      using rule_t = failure;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& /*unused*/ ) noexcept
      {
         return false;
      }
   };

   template<>
   inline constexpr bool enable_control< failure > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 24 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/identifier.hpp"
       
#line 1 "tao/pegtl/internal/identifier.hpp"



#ifndef TAO_PEGTL_INTERNAL_IDENTIFIER_HPP
#define TAO_PEGTL_INTERNAL_IDENTIFIER_HPP




#line 1 "tao/pegtl/internal/ranges.hpp"
       
#line 1 "tao/pegtl/internal/ranges.hpp"



#ifndef TAO_PEGTL_INTERNAL_RANGES_HPP
#define TAO_PEGTL_INTERNAL_RANGES_HPP





#line 1 "tao/pegtl/internal/one.hpp"
       
#line 1 "tao/pegtl/internal/one.hpp"



#ifndef TAO_PEGTL_INTERNAL_ONE_HPP
#define TAO_PEGTL_INTERNAL_ONE_HPP

#include <cstddef>




#line 1 "tao/pegtl/internal/bump_help.hpp"
       
#line 1 "tao/pegtl/internal/bump_help.hpp"



#ifndef TAO_PEGTL_INTERNAL_BUMP_HELP_HPP
#define TAO_PEGTL_INTERNAL_BUMP_HELP_HPP

#include <cstddef>
#include <type_traits>





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< result_on_found R, typename ParseInput, typename Char, Char... Cs >
   void bump_help( ParseInput& in, const std::size_t count ) noexcept
   {
      if constexpr( ( ( Cs != ParseInput::eol_t::ch ) && ... ) != bool( R ) ) {
         in.bump( count );
      }
      else {
         in.bump_in_this_line( count );
      }
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/one.hpp"






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< result_on_found R, typename Peek, typename Peek::data_t... Cs >
   struct one
   {
      using rule_t = one;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( Peek::peek( in ) ) )
      {
         if( const auto t = Peek::peek( in ) ) {
            if( ( ( t.data == Cs ) || ... ) == bool( R ) ) {
               bump_help< R, ParseInput, typename Peek::data_t, Cs... >( in, t.size );
               return true;
            }
         }
         return false;
      }
   };

   template< typename Peek >
   struct one< result_on_found::success, Peek >
      : failure
   {};

   template< typename Peek >
   struct one< result_on_found::failure, Peek >
      : any< Peek >
   {};

   template< result_on_found R, typename Peek, typename Peek::data_t... Cs >
   inline constexpr bool enable_control< one< R, Peek, Cs... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/ranges.hpp"
#line 1 "tao/pegtl/internal/range.hpp"
       
#line 1 "tao/pegtl/internal/range.hpp"



#ifndef TAO_PEGTL_INTERNAL_RANGE_HPP
#define TAO_PEGTL_INTERNAL_RANGE_HPP
#line 15 "tao/pegtl/internal/range.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< result_on_found R, typename Peek, typename Peek::data_t Lo, typename Peek::data_t Hi >
   struct range
   {
      using rule_t = range;
      using subs_t = empty_list;

      static_assert( Lo < Hi, "invalid range" );

      template< int Eol >
      static constexpr bool can_match_eol = ( ( ( Lo <= Eol ) && ( Eol <= Hi ) ) == bool( R ) );

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( Peek::peek( in ) ) )
      {
         if( const auto t = Peek::peek( in ) ) {
            if( ( ( Lo <= t.data ) && ( t.data <= Hi ) ) == bool( R ) ) {
               if constexpr( can_match_eol< ParseInput::eol_t::ch > ) {
                  in.bump( t.size );
               }
               else {
                  in.bump_in_this_line( t.size );
               }
               return true;
            }
         }
         return false;
      }
   };

   template< result_on_found R, typename Peek, typename Peek::data_t C >
   struct range< R, Peek, C, C >
      : one< R, Peek, C >
   {};

   template< result_on_found R, typename Peek, typename Peek::data_t Lo, typename Peek::data_t Hi >
   inline constexpr bool enable_control< range< R, Peek, Lo, Hi > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/ranges.hpp"



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< int Eol, typename Char, Char... Cs >
   struct ranges_impl;

   template< int Eol, typename Char >
   struct ranges_impl< Eol, Char >
   {
      static constexpr bool can_match_eol = false;

      [[nodiscard]] static bool match( const Char /*unused*/ ) noexcept
      {
         return false;
      }
   };

   template< int Eol, typename Char, Char Eq >
   struct ranges_impl< Eol, Char, Eq >
   {
      static constexpr bool can_match_eol = ( Eq == Eol );

      [[nodiscard]] static bool match( const Char c ) noexcept
      {
         return c == Eq;
      }
   };

   template< int Eol, typename Char, Char Lo, Char Hi, Char... Cs >
   struct ranges_impl< Eol, Char, Lo, Hi, Cs... >
   {
      static_assert( Lo <= Hi, "invalid range detected" );

      static constexpr bool can_match_eol = ( ( ( Lo <= Eol ) && ( Eol <= Hi ) ) || ranges_impl< Eol, Char, Cs... >::can_match_eol );

      [[nodiscard]] static bool match( const Char c ) noexcept
      {
         return ( ( Lo <= c ) && ( c <= Hi ) ) || ranges_impl< Eol, Char, Cs... >::match( c );
      }
   };

   template< typename Peek, typename Peek::data_t... Cs >
   struct ranges
   {
      using rule_t = ranges;
      using subs_t = empty_list;

      template< int Eol >
      static constexpr bool can_match_eol = ranges_impl< Eol, typename Peek::data_t, Cs... >::can_match_eol;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( Peek::peek( in ) ) )
      {
         if( const auto t = Peek::peek( in ) ) {
            if( ranges_impl< ParseInput::eol_t::ch, typename Peek::data_t, Cs... >::match( t.data ) ) {
               if constexpr( can_match_eol< ParseInput::eol_t::ch > ) {
                  in.bump( t.size );
               }
               else {
                  in.bump_in_this_line( t.size );
               }
               return true;
            }
         }
         return false;
      }
   };

   template< typename Peek, typename Peek::data_t Lo, typename Peek::data_t Hi >
   struct ranges< Peek, Lo, Hi >
      : range< result_on_found::success, Peek, Lo, Hi >
   {};

   template< typename Peek, typename Peek::data_t C >
   struct ranges< Peek, C >
      : one< result_on_found::success, Peek, C >
   {};

   template< typename Peek >
   struct ranges< Peek >
      : failure
   {};

   template< typename Peek, typename Peek::data_t... Cs >
   inline constexpr bool enable_control< ranges< Peek, Cs... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/identifier.hpp"

#line 1 "tao/pegtl/internal/star.hpp"
       
#line 1 "tao/pegtl/internal/star.hpp"



#ifndef TAO_PEGTL_INTERNAL_STAR_HPP
#define TAO_PEGTL_INTERNAL_STAR_HPP

#include <type_traits>
#line 18 "tao/pegtl/internal/star.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename... Rules >
   struct star
      : star< seq< Rule, Rules... > >
   {};

   template< typename Rule >
   struct star< Rule >
   {
      using rule_t = star;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         while( Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
         }
         return true;
      }
   };

   template< typename Rule, typename... Rules >
   inline constexpr bool enable_control< star< Rule, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/identifier.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   using identifier_first = ranges< peek_char, 'a', 'z', 'A', 'Z', '_' >;
   using identifier_other = ranges< peek_char, 'a', 'z', 'A', 'Z', '0', '9', '_' >;
   using identifier = seq< identifier_first, star< identifier_other > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 25 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/if_apply.hpp"
       
#line 1 "tao/pegtl/internal/if_apply.hpp"



#ifndef TAO_PEGTL_INTERNAL_IF_APPLY_HPP
#define TAO_PEGTL_INTERNAL_IF_APPLY_HPP
#line 16 "tao/pegtl/internal/if_apply.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename... Actions >
   struct if_apply
   {
      using rule_t = if_apply;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( ( A == apply_mode::action ) && ( sizeof...( Actions ) != 0 ) ) {
            using action_t = typename ParseInput::action_t;
            auto m = in.template mark< rewind_mode::required >();
            if( Control< Rule >::template match< apply_mode::action, rewind_mode::active, Action, Control >( in, st... ) ) {
               const action_t i2( m.iterator(), in );
               return m( ( apply_single< Actions >::match( i2, st... ) && ... ) );
            }
            return false;
         }
         else {
            return Control< Rule >::template match< A, M, Action, Control >( in, st... );
         }
      }
   };

   template< typename Rule, typename... Actions >
   inline constexpr bool enable_control< if_apply< Rule, Actions... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 26 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/if_must.hpp"
       
#line 1 "tao/pegtl/internal/if_must.hpp"



#ifndef TAO_PEGTL_INTERNAL_IF_MUST_HPP
#define TAO_PEGTL_INTERNAL_IF_MUST_HPP




#line 1 "tao/pegtl/internal/must.hpp"
       
#line 1 "tao/pegtl/internal/must.hpp"



#ifndef TAO_PEGTL_INTERNAL_MUST_HPP
#define TAO_PEGTL_INTERNAL_MUST_HPP
#line 17 "tao/pegtl/internal/must.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   // The general case applies must<> to each of the
   // rules in the 'Rules' parameter pack individually.

   template< typename... Rules >
   struct must
      : seq< must< Rules >... >
   {};

   template<>
   struct must<>
      : success
   {};

   // While in theory the implementation for a single rule could
   // be simplified to must< Rule > = sor< Rule, raise< Rule > >, this
   // would result in some unnecessary run-time overhead.

   template< typename Rule >
   struct must< Rule >
   {
      using rule_t = must;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if( !Control< Rule >::template match< A, rewind_mode::dontcare, Action, Control >( in, st... ) ) {
            Control< Rule >::raise( static_cast< const ParseInput& >( in ), st... );
         }
         return true;
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< must< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/if_must.hpp"





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< bool Default, typename Cond, typename... Rules >
   struct if_must
   {
      using rule_t = if_must;
      using subs_t = type_list< Cond, must< Rules... > >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if( Control< Cond >::template match< A, M, Action, Control >( in, st... ) ) {
            (void)Control< must< Rules... > >::template match< A, M, Action, Control >( in, st... );
            return true;
         }
         return Default;
      }
   };

   template< bool Default, typename Cond, typename... Rules >
   inline constexpr bool enable_control< if_must< Default, Cond, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 27 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/if_must_else.hpp"
       
#line 1 "tao/pegtl/internal/if_must_else.hpp"



#ifndef TAO_PEGTL_INTERNAL_IF_MUST_ELSE_HPP
#define TAO_PEGTL_INTERNAL_IF_MUST_ELSE_HPP



#line 1 "tao/pegtl/internal/if_then_else.hpp"
       
#line 1 "tao/pegtl/internal/if_then_else.hpp"



#ifndef TAO_PEGTL_INTERNAL_IF_THEN_ELSE_HPP
#define TAO_PEGTL_INTERNAL_IF_THEN_ELSE_HPP




#line 1 "tao/pegtl/internal/not_at.hpp"
       
#line 1 "tao/pegtl/internal/not_at.hpp"



#ifndef TAO_PEGTL_INTERNAL_NOT_AT_HPP
#define TAO_PEGTL_INTERNAL_NOT_AT_HPP
#line 17 "tao/pegtl/internal/not_at.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct not_at
      : not_at< seq< Rules... > >
   {};

   template<>
   struct not_at<>
      : failure
   {};

   template< typename Rule >
   struct not_at< Rule >
   {
      using rule_t = not_at;
      using subs_t = type_list< Rule >;

      template< apply_mode,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         const auto m = in.template mark< rewind_mode::required >();
         return !Control< Rule >::template match< apply_mode::nothing, rewind_mode::active, Action, Control >( in, st... );
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< not_at< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 11 "tao/pegtl/internal/if_then_else.hpp"

#line 1 "tao/pegtl/internal/sor.hpp"
       
#line 1 "tao/pegtl/internal/sor.hpp"



#ifndef TAO_PEGTL_INTERNAL_SOR_HPP
#define TAO_PEGTL_INTERNAL_SOR_HPP

#include <utility>
#line 18 "tao/pegtl/internal/sor.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct sor;

   template<>
   struct sor<>
      : failure
   {};

   template< typename... Rules >
   struct sor
   {
      using rule_t = sor;
      using subs_t = type_list< Rules... >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                std::size_t... Indices,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( std::index_sequence< Indices... > /*unused*/, ParseInput& in, States&&... st )
      {
         return ( Control< Rules >::template match< A, ( ( Indices == ( sizeof...( Rules ) - 1 ) ) ? M : rewind_mode::required ), Action, Control >( in, st... ) || ... );
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return match< A, M, Action, Control >( std::index_sequence_for< Rules... >(), in, st... );
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< sor< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/if_then_else.hpp"





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Cond, typename Then, typename Else >
   struct if_then_else
   {
      using rule_t = if_then_else;
      using subs_t = type_list< Cond, Then, Else >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();
         using m_t = decltype( m );

         if( Control< Cond >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
            return m( Control< Then >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) );
         }
         return m( Control< Else >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) );
      }
   };

   template< typename Cond, typename Then, typename Else >
   inline constexpr bool enable_control< if_then_else< Cond, Then, Else > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/internal/if_must_else.hpp"




namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Cond, typename Then, typename Else >
   using if_must_else = if_then_else< Cond, must< Then >, must< Else > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 28 "tao/pegtl/internal/rules.hpp"

#line 1 "tao/pegtl/internal/istring.hpp"
       
#line 1 "tao/pegtl/internal/istring.hpp"



#ifndef TAO_PEGTL_INTERNAL_ISTRING_HPP
#define TAO_PEGTL_INTERNAL_ISTRING_HPP

#include <type_traits>
#line 18 "tao/pegtl/internal/istring.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< char C >
   inline constexpr bool is_alpha = ( ( 'a' <= C ) && ( C <= 'z' ) ) || ( ( 'A' <= C ) && ( C <= 'Z' ) );

   template< char C >
   [[nodiscard]] bool ichar_equal( const char c ) noexcept
   {
      if constexpr( is_alpha< C > ) {
         return ( C | 0x20 ) == ( c | 0x20 );
      }
      else {
         return c == C;
      }
   }

   template< char... Cs >
   [[nodiscard]] bool istring_equal( const char* r ) noexcept
   {
      return ( ichar_equal< Cs >( *r++ ) && ... );
   }

   template< char... Cs >
   struct istring;

   template<>
   struct istring<>
      : success
   {};

   template< char... Cs >
   struct istring
   {
      using rule_t = istring;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.size( 0 ) ) )
      {
         if( in.size( sizeof...( Cs ) ) >= sizeof...( Cs ) ) {
            if( istring_equal< Cs... >( in.current() ) ) {
               bump_help< result_on_found::success, ParseInput, char, Cs... >( in, sizeof...( Cs ) );
               return true;
            }
         }
         return false;
      }
   };

   template< char... Cs >
   inline constexpr bool enable_control< istring< Cs... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 30 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/list.hpp"
       
#line 1 "tao/pegtl/internal/list.hpp"



#ifndef TAO_PEGTL_INTERNAL_LIST_HPP
#define TAO_PEGTL_INTERNAL_LIST_HPP






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Sep >
   using list = seq< Rule, star< Sep, Rule > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 31 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/list_must.hpp"
       
#line 1 "tao/pegtl/internal/list_must.hpp"



#ifndef TAO_PEGTL_INTERNAL_LIST_MUST_HPP
#define TAO_PEGTL_INTERNAL_LIST_MUST_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Sep >
   using list_must = seq< Rule, star< Sep, must< Rule > > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 32 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/list_tail.hpp"
       
#line 1 "tao/pegtl/internal/list_tail.hpp"



#ifndef TAO_PEGTL_INTERNAL_LIST_TAIL_HPP
#define TAO_PEGTL_INTERNAL_LIST_TAIL_HPP



#line 1 "tao/pegtl/internal/opt.hpp"
       
#line 1 "tao/pegtl/internal/opt.hpp"



#ifndef TAO_PEGTL_INTERNAL_OPT_HPP
#define TAO_PEGTL_INTERNAL_OPT_HPP

#include <type_traits>
#line 19 "tao/pegtl/internal/opt.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Rules >
   struct opt
      : opt< seq< Rules... > >
   {};

   template<>
   struct opt<>
      : success
   {};

   template< typename Rule >
   struct opt< Rule >
   {
      using rule_t = opt;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         (void)Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... );
         return true;
      }
   };

   template< typename... Rules >
   inline constexpr bool enable_control< opt< Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/internal/list_tail.hpp"





namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Sep >
   using list_tail = seq< Rule, star< Sep, Rule >, opt< Sep > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 33 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/list_tail_pad.hpp"
       
#line 1 "tao/pegtl/internal/list_tail_pad.hpp"



#ifndef TAO_PEGTL_INTERNAL_LIST_TAIL_PAD_HPP
#define TAO_PEGTL_INTERNAL_LIST_TAIL_PAD_HPP





#line 1 "tao/pegtl/internal/pad.hpp"
       
#line 1 "tao/pegtl/internal/pad.hpp"



#ifndef TAO_PEGTL_INTERNAL_PAD_HPP
#define TAO_PEGTL_INTERNAL_PAD_HPP






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Pad1, typename Pad2 = Pad1 >
   using pad = seq< star< Pad1 >, Rule, star< Pad2 > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/list_tail_pad.hpp"



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Sep, typename Pad >
   using list_tail_pad = seq< Rule, star< pad< Sep, Pad >, Rule >, opt< star< Pad >, Sep > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 34 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/minus.hpp"
       
#line 1 "tao/pegtl/internal/minus.hpp"



#ifndef TAO_PEGTL_INTERNAL_MINUS_HPP
#define TAO_PEGTL_INTERNAL_MINUS_HPP





#line 1 "tao/pegtl/internal/rematch.hpp"
       
#line 1 "tao/pegtl/internal/rematch.hpp"



#ifndef TAO_PEGTL_INTERNAL_REMATCH_HPP
#define TAO_PEGTL_INTERNAL_REMATCH_HPP






#line 1 "tao/pegtl/internal/../memory_input.hpp"
       
#line 1 "tao/pegtl/internal/../memory_input.hpp"



#ifndef TAO_PEGTL_MEMORY_INPUT_HPP
#define TAO_PEGTL_MEMORY_INPUT_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>


#line 1 "tao/pegtl/internal/../eol.hpp"
       
#line 1 "tao/pegtl/internal/../eol.hpp"



#ifndef TAO_PEGTL_EOL_HPP
#define TAO_PEGTL_EOL_HPP





#line 1 "tao/pegtl/internal/../internal/cr_crlf_eol.hpp"
       
#line 1 "tao/pegtl/internal/../internal/cr_crlf_eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_CR_CRLF_EOL_HPP
#define TAO_PEGTL_INTERNAL_CR_CRLF_EOL_HPP


#line 1 "tao/pegtl/internal/../internal/../eol_pair.hpp"
       
#line 1 "tao/pegtl/internal/../internal/../eol_pair.hpp"



#ifndef TAO_PEGTL_EOL_PAIR_HPP
#define TAO_PEGTL_EOL_PAIR_HPP

#include <cstddef>
#include <utility>



namespace TAO_PEGTL_NAMESPACE
{
   using eol_pair = std::pair< bool, std::size_t >;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 9 "tao/pegtl/internal/../internal/cr_crlf_eol.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   struct cr_crlf_eol
   {
      static constexpr int ch = '\r';

      template< typename ParseInput >
      [[nodiscard]] static eol_pair match( ParseInput& in ) noexcept( noexcept( in.size( 2 ) ) )
      {
         eol_pair p = { false, in.size( 2 ) };
         if( p.second ) {
            if( in.peek_char() == '\r' ) {
               in.bump_to_next_line( 1 + ( ( p.second > 1 ) && ( in.peek_char( 1 ) == '\n' ) ) );
               p.first = true;
            }
         }
         return p;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/../eol.hpp"
#line 1 "tao/pegtl/internal/../internal/cr_eol.hpp"
       
#line 1 "tao/pegtl/internal/../internal/cr_eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_CR_EOL_HPP
#define TAO_PEGTL_INTERNAL_CR_EOL_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   struct cr_eol
   {
      static constexpr int ch = '\r';

      template< typename ParseInput >
      [[nodiscard]] static eol_pair match( ParseInput& in ) noexcept( noexcept( in.size( 1 ) ) )
      {
         eol_pair p = { false, in.size( 1 ) };
         if( p.second ) {
            if( in.peek_char() == '\r' ) {
               in.bump_to_next_line();
               p.first = true;
            }
         }
         return p;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/internal/../eol.hpp"
#line 1 "tao/pegtl/internal/../internal/crlf_eol.hpp"
       
#line 1 "tao/pegtl/internal/../internal/crlf_eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_CRLF_EOL_HPP
#define TAO_PEGTL_INTERNAL_CRLF_EOL_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   struct crlf_eol
   {
      static constexpr int ch = '\n';

      template< typename ParseInput >
      [[nodiscard]] static eol_pair match( ParseInput& in ) noexcept( noexcept( in.size( 2 ) ) )
      {
         eol_pair p = { false, in.size( 2 ) };
         if( p.second > 1 ) {
            if( ( in.peek_char() == '\r' ) && ( in.peek_char( 1 ) == '\n' ) ) {
               in.bump_to_next_line( 2 );
               p.first = true;
            }
         }
         return p;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/internal/../eol.hpp"
#line 1 "tao/pegtl/internal/../internal/lf_crlf_eol.hpp"
       
#line 1 "tao/pegtl/internal/../internal/lf_crlf_eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_LF_CRLF_EOL_HPP
#define TAO_PEGTL_INTERNAL_LF_CRLF_EOL_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   struct lf_crlf_eol
   {
      static constexpr int ch = '\n';

      template< typename ParseInput >
      [[nodiscard]] static eol_pair match( ParseInput& in ) noexcept( noexcept( in.size( 2 ) ) )
      {
         eol_pair p = { false, in.size( 2 ) };
         if( p.second ) {
            const auto a = in.peek_char();
            if( a == '\n' ) {
               in.bump_to_next_line();
               p.first = true;
            }
            else if( ( a == '\r' ) && ( p.second > 1 ) && ( in.peek_char( 1 ) == '\n' ) ) {
               in.bump_to_next_line( 2 );
               p.first = true;
            }
         }
         return p;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 15 "tao/pegtl/internal/../eol.hpp"
#line 1 "tao/pegtl/internal/../internal/lf_eol.hpp"
       
#line 1 "tao/pegtl/internal/../internal/lf_eol.hpp"



#ifndef TAO_PEGTL_INTERNAL_LF_EOL_HPP
#define TAO_PEGTL_INTERNAL_LF_EOL_HPP




namespace TAO_PEGTL_NAMESPACE::internal
{
   struct lf_eol
   {
      static constexpr int ch = '\n';

      template< typename ParseInput >
      [[nodiscard]] static eol_pair match( ParseInput& in ) noexcept( noexcept( in.size( 1 ) ) )
      {
         eol_pair p = { false, in.size( 1 ) };
         if( p.second ) {
            if( in.peek_char() == '\n' ) {
               in.bump_to_next_line();
               p.first = true;
            }
         }
         return p;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 16 "tao/pegtl/internal/../eol.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   inline namespace ascii
   {
      // this is both a rule and a pseudo-namespace for eol::cr, ...
      struct eol : internal::eol
      {
         // clang-format off
         struct cr : internal::cr_eol {};
         struct cr_crlf : internal::cr_crlf_eol {};
         struct crlf : internal::crlf_eol {};
         struct lf : internal::lf_eol {};
         struct lf_crlf : internal::lf_crlf_eol {};
         // clang-format on
      };

   } // namespace ascii

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 18 "tao/pegtl/internal/../memory_input.hpp"



#line 1 "tao/pegtl/internal/../tracking_mode.hpp"
       
#line 1 "tao/pegtl/internal/../tracking_mode.hpp"



#ifndef TAO_PEGTL_TRACKING_MODE_HPP
#define TAO_PEGTL_TRACKING_MODE_HPP



namespace TAO_PEGTL_NAMESPACE
{
   enum class tracking_mode : bool
   {
      eager,
      lazy
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 22 "tao/pegtl/internal/../memory_input.hpp"

#line 1 "tao/pegtl/internal/../internal/action_input.hpp"
       
#line 1 "tao/pegtl/internal/../internal/action_input.hpp"



#ifndef TAO_PEGTL_INTERNAL_ACTION_INPUT_HPP
#define TAO_PEGTL_INTERNAL_ACTION_INPUT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename ParseInput >
   class action_input
   {
   public:
      using input_t = ParseInput;
      using iterator_t = typename ParseInput::iterator_t;

      action_input( const iterator_t& in_begin, const ParseInput& in_input ) noexcept
         : m_begin( in_begin ),
           m_input( in_input )
      {}

      action_input( const action_input& ) = delete;
      action_input( action_input&& ) = delete;

      ~action_input() = default;

      action_input& operator=( const action_input& ) = delete;
      action_input& operator=( action_input&& ) = delete;

      [[nodiscard]] const iterator_t& iterator() const noexcept
      {
         return m_begin;
      }

      [[nodiscard]] const ParseInput& input() const noexcept
      {
         return m_input;
      }

      [[nodiscard]] const char* begin() const noexcept
      {
         if constexpr( std::is_same_v< iterator_t, const char* > ) {
            return iterator();
         }
         else {
            return iterator().data;
         }
      }

      [[nodiscard]] const char* end() const noexcept
      {
         return input().current();
      }

      [[nodiscard]] bool empty() const noexcept
      {
         return begin() == end();
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return std::size_t( end() - begin() );
      }

      [[nodiscard]] std::string string() const
      {
         return std::string( begin(), size() );
      }

      [[nodiscard]] std::string_view string_view() const noexcept
      {
         return std::string_view( begin(), size() );
      }

      [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept
      {
         return begin()[ offset ];
      }

      [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept
      {
         return static_cast< std::uint8_t >( peek_char( offset ) );
      }

      [[nodiscard]] TAO_PEGTL_NAMESPACE::position position() const
      {
         return input().position( iterator() ); // NOTE: Not efficient with lazy inputs.
      }

   protected:
      const iterator_t m_begin;
      const ParseInput& m_input;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 24 "tao/pegtl/internal/../memory_input.hpp"

#line 1 "tao/pegtl/internal/../internal/bump.hpp"
       
#line 1 "tao/pegtl/internal/../internal/bump.hpp"



#ifndef TAO_PEGTL_INTERNAL_BUMP_HPP
#define TAO_PEGTL_INTERNAL_BUMP_HPP





namespace TAO_PEGTL_NAMESPACE::internal
{
   inline void bump( iterator& iter, const std::size_t count, const int ch ) noexcept
   {
      for( std::size_t i = 0; i < count; ++i ) {
         if( iter.data[ i ] == ch ) {
            ++iter.line;
            iter.column = 1;
         }
         else {
            ++iter.column;
         }
      }
      iter.byte += count;
      iter.data += count;
   }

   inline void bump_in_this_line( iterator& iter, const std::size_t count ) noexcept
   {
      iter.data += count;
      iter.byte += count;
      iter.column += count;
   }

   inline void bump_to_next_line( iterator& iter, const std::size_t count ) noexcept
   {
      ++iter.line;
      iter.byte += count;
      iter.column = 1;
      iter.data += count;
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 26 "tao/pegtl/internal/../memory_input.hpp"



#line 1 "tao/pegtl/internal/../internal/until.hpp"
       
#line 1 "tao/pegtl/internal/../internal/until.hpp"



#ifndef TAO_PEGTL_INTERNAL_UNTIL_HPP
#define TAO_PEGTL_INTERNAL_UNTIL_HPP
#line 20 "tao/pegtl/internal/../internal/until.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Cond, typename... Rules >
   struct until
      : until< Cond, seq< Rules... > >
   {};

   template< typename Cond >
   struct until< Cond >
   {
      using rule_t = until;
      using subs_t = type_list< Cond >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();

         while( !Control< Cond >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
            if( in.empty() ) {
               return false;
            }
            in.bump();
         }
         return m( true );
      }
   };

   template< typename Cond, typename Rule >
   struct until< Cond, Rule >
   {
      using rule_t = until;
      using subs_t = type_list< Cond, Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();
         using m_t = decltype( m );

         while( !Control< Cond >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
            if( !Control< Rule >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) ) {
               return false;
            }
         }
         return m( true );
      }
   };

   template< typename Cond, typename... Rules >
   inline constexpr bool enable_control< until< Cond, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 30 "tao/pegtl/internal/../memory_input.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< tracking_mode, typename Eol, typename Source >
      class memory_input_base;

      template< typename Eol, typename Source >
      class memory_input_base< tracking_mode::eager, Eol, Source >
      {
      public:
         using iterator_t = internal::iterator;

         template< typename T >
         memory_input_base( const iterator_t& in_begin, const char* in_end, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
            : m_begin( in_begin.data ),
              m_current( in_begin ),
              m_end( in_end ),
              m_source( std::forward< T >( in_source ) )
         {}

         template< typename T >
         memory_input_base( const char* in_begin, const char* in_end, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
            : m_begin( in_begin ),
              m_current( in_begin ),
              m_end( in_end ),
              m_source( std::forward< T >( in_source ) )
         {}

         memory_input_base( const memory_input_base& ) = delete;
         memory_input_base( memory_input_base&& ) = delete;

         ~memory_input_base() = default;

         memory_input_base operator=( const memory_input_base& ) = delete;
         memory_input_base operator=( memory_input_base&& ) = delete;

         [[nodiscard]] const char* current() const noexcept
         {
            return m_current.data;
         }

         [[nodiscard]] const char* begin() const noexcept
         {
            return m_begin;
         }

         [[nodiscard]] const char* end( const std::size_t /*unused*/ = 0 ) const noexcept
         {
            return m_end;
         }

         [[nodiscard]] std::size_t byte() const noexcept
         {
            return m_current.byte;
         }

         [[nodiscard]] std::size_t line() const noexcept
         {
            return m_current.line;
         }

         [[nodiscard]] std::size_t column() const noexcept
         {
            return m_current.column;
         }

         void bump( const std::size_t in_count = 1 ) noexcept
         {
            internal::bump( m_current, in_count, Eol::ch );
         }

         void bump_in_this_line( const std::size_t in_count = 1 ) noexcept
         {
            internal::bump_in_this_line( m_current, in_count );
         }

         void bump_to_next_line( const std::size_t in_count = 1 ) noexcept
         {
            internal::bump_to_next_line( m_current, in_count );
         }

         [[nodiscard]] TAO_PEGTL_NAMESPACE::position position( const iterator_t& it ) const
         {
            return TAO_PEGTL_NAMESPACE::position( it, m_source );
         }

         void restart( const std::size_t in_byte = 0, const std::size_t in_line = 1, const std::size_t in_column = 1 )
         {
            assert( in_line != 0 );
            assert( in_column != 0 );

            m_current.data = m_begin;
            m_current.byte = in_byte;
            m_current.line = in_line;
            m_current.column = in_column;
         }

      protected:
         const char* const m_begin;
         iterator_t m_current;
         const char* const m_end;
         const Source m_source;
      };

      template< typename Eol, typename Source >
      class memory_input_base< tracking_mode::lazy, Eol, Source >
      {
      public:
         using iterator_t = const char*;

         template< typename T >
         memory_input_base( const internal::iterator& in_begin, const char* in_end, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
            : m_begin( in_begin ),
              m_current( in_begin.data ),
              m_end( in_end ),
              m_source( std::forward< T >( in_source ) )
         {}

         template< typename T >
         memory_input_base( const char* in_begin, const char* in_end, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
            : m_begin( in_begin ),
              m_current( in_begin ),
              m_end( in_end ),
              m_source( std::forward< T >( in_source ) )
         {}

         memory_input_base( const memory_input_base& ) = delete;
         memory_input_base( memory_input_base&& ) = delete;

         ~memory_input_base() = default;

         memory_input_base operator=( const memory_input_base& ) = delete;
         memory_input_base operator=( memory_input_base&& ) = delete;

         [[nodiscard]] const char* current() const noexcept
         {
            return m_current;
         }

         [[nodiscard]] const char* begin() const noexcept
         {
            return m_begin.data;
         }

         [[nodiscard]] const char* end( const std::size_t /*unused*/ = 0 ) const noexcept
         {
            return m_end;
         }

         [[nodiscard]] std::size_t byte() const noexcept
         {
            return std::size_t( current() - m_begin.data );
         }

         void bump( const std::size_t in_count = 1 ) noexcept
         {
            m_current += in_count;
         }

         void bump_in_this_line( const std::size_t in_count = 1 ) noexcept
         {
            m_current += in_count;
         }

         void bump_to_next_line( const std::size_t in_count = 1 ) noexcept
         {
            m_current += in_count;
         }

         [[nodiscard]] TAO_PEGTL_NAMESPACE::position position( const iterator_t it ) const
         {
            internal::iterator c( m_begin );
            internal::bump( c, std::size_t( it - m_begin.data ), Eol::ch );
            return TAO_PEGTL_NAMESPACE::position( c, m_source );
         }

         void restart()
         {
            m_current = m_begin.data;
         }

      protected:
         const internal::iterator m_begin;
         iterator_t m_current;
         const char* const m_end;
         const Source m_source;
      };

   } // namespace internal

   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf, typename Source = std::string >
   class memory_input
      : public internal::memory_input_base< P, Eol, Source >
   {
   public:
      static constexpr tracking_mode tracking_mode_v = P;

      using eol_t = Eol;
      using source_t = Source;

      using typename internal::memory_input_base< P, Eol, Source >::iterator_t;

      using action_t = internal::action_input< memory_input >;

      using internal::memory_input_base< P, Eol, Source >::memory_input_base;

      template< typename T >
      memory_input( const char* in_begin, const std::size_t in_size, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
         : memory_input( in_begin, in_begin + in_size, std::forward< T >( in_source ) )
      {}

      template< typename T >
      memory_input( const std::string& in_string, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
         : memory_input( in_string.data(), in_string.size(), std::forward< T >( in_source ) )
      {}

      template< typename T >
      memory_input( const std::string_view in_string, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
         : memory_input( in_string.data(), in_string.size(), std::forward< T >( in_source ) )
      {}

      template< typename T >
      memory_input( std::string&&, T&& ) = delete;

      template< typename T >
      memory_input( const char* in_begin, T&& in_source ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
         : memory_input( in_begin, std::strlen( in_begin ), std::forward< T >( in_source ) )
      {}

      template< typename T >
      memory_input( const char* in_begin, const char* in_end, T&& in_source, const std::size_t in_byte, const std::size_t in_line, const std::size_t in_column ) noexcept( std::is_nothrow_constructible_v< Source, T&& > )
         : memory_input( { in_begin, in_byte, in_line, in_column }, in_end, std::forward< T >( in_source ) )
      {}

      memory_input( const memory_input& ) = delete;
      memory_input( memory_input&& ) = delete;

      ~memory_input() = default;

      memory_input operator=( const memory_input& ) = delete;
      memory_input operator=( memory_input&& ) = delete;

      [[nodiscard]] const Source& source() const noexcept
      {
         return this->m_source;
      }

      [[nodiscard]] bool empty() const noexcept
      {
         return this->current() == this->end();
      }

      [[nodiscard]] std::size_t size( const std::size_t /*unused*/ = 0 ) const noexcept
      {
         return std::size_t( this->end() - this->current() );
      }

      [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept
      {
         return this->current()[ offset ];
      }

      [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept
      {
         return static_cast< std::uint8_t >( peek_char( offset ) );
      }

      [[nodiscard]] iterator_t& iterator() noexcept
      {
         return this->m_current;
      }

      [[nodiscard]] const iterator_t& iterator() const noexcept
      {
         return this->m_current;
      }

      using internal::memory_input_base< P, Eol, Source >::restart;

      template< rewind_mode M >
      void restart( const internal::marker< iterator_t, M >& m ) noexcept
      {
         iterator() = m.iterator();
      }

      using internal::memory_input_base< P, Eol, Source >::position;

      [[nodiscard]] TAO_PEGTL_NAMESPACE::position position() const
      {
         return position( iterator() );
      }

      void discard() const noexcept {}

      void require( const std::size_t /*unused*/ ) const noexcept {}

      template< rewind_mode M >
      [[nodiscard]] internal::marker< iterator_t, M > mark() noexcept
      {
         return internal::marker< iterator_t, M >( iterator() );
      }

      [[nodiscard]] const char* at( const TAO_PEGTL_NAMESPACE::position& p ) const noexcept
      {
         return this->begin() + p.byte;
      }

      [[nodiscard]] const char* begin_of_line( const TAO_PEGTL_NAMESPACE::position& p ) const noexcept
      {
         return at( p ) - ( p.column - 1 );
      }

      [[nodiscard]] const char* end_of_line( const TAO_PEGTL_NAMESPACE::position& p ) const noexcept
      {
         using input_t = memory_input< tracking_mode::lazy, Eol, const char* >;
         input_t in( at( p ), this->end(), "" );
         using grammar = internal::until< internal::at< internal::eolf > >;
         (void)normal< grammar >::match< apply_mode::nothing, rewind_mode::dontcare, nothing, normal >( in );
         return in.current();
      }

      [[nodiscard]] std::string_view line_at( const TAO_PEGTL_NAMESPACE::position& p ) const noexcept
      {
         const char* b = begin_of_line( p );
         return std::string_view( b, static_cast< std::size_t >( end_of_line( p ) - b ) );
      }
   };

   template< typename... Ts >
   memory_input( Ts&&... ) -> memory_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 13 "tao/pegtl/internal/rematch.hpp"



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Head, typename... Rules >
   struct rematch;

   template< typename Head >
   struct rematch< Head >
   {
      using rule_t = rematch;
      using subs_t = type_list< Head >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return Control< Head >::template match< A, M, Action, Control >( in, st... );
      }
   };

   template< typename Head, typename Rule, typename... Rules >
   struct rematch< Head, Rule, Rules... >
   {
      using rule_t = rematch;
      using subs_t = type_list< Head, Rule, Rules... >;

      template< apply_mode A,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< rewind_mode::required >();

         if( Control< Head >::template match< A, rewind_mode::active, Action, Control >( in, st... ) ) {
            memory_input< ParseInput::tracking_mode_v, typename ParseInput::eol_t, typename ParseInput::source_t > i2( m.iterator(), in.current(), in.source() );
            return m( ( Control< Rule >::template match< A, rewind_mode::active, Action, Control >( i2, st... ) && ... && ( i2.restart( m ), Control< Rules >::template match< A, rewind_mode::active, Action, Control >( i2, st... ) ) ) );
         }
         return false;
      }
   };

   template< typename Head, typename... Rules >
   inline constexpr bool enable_control< rematch< Head, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/internal/minus.hpp"


namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename M, typename S >
   using minus = rematch< M, not_at< S, eof > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 35 "tao/pegtl/internal/rules.hpp"





#line 1 "tao/pegtl/internal/pad_opt.hpp"
       
#line 1 "tao/pegtl/internal/pad_opt.hpp"



#ifndef TAO_PEGTL_INTERNAL_PAD_OPT_HPP
#define TAO_PEGTL_INTERNAL_PAD_OPT_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Pad >
   using pad_opt = seq< star< Pad >, opt< Rule, star< Pad > > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 41 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/plus.hpp"
       
#line 1 "tao/pegtl/internal/plus.hpp"



#ifndef TAO_PEGTL_INTERNAL_PLUS_HPP
#define TAO_PEGTL_INTERNAL_PLUS_HPP

#include <type_traits>
#line 18 "tao/pegtl/internal/plus.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   // While plus<> could easily be implemented with
   // seq< Rule, Rules ..., star< Rule, Rules ... > > we
   // provide an explicit implementation to optimise away
   // the otherwise created input mark.

   template< typename Rule, typename... Rules >
   struct plus
      : plus< seq< Rule, Rules... > >
   {};

   template< typename Rule >
   struct plus< Rule >
   {
      using rule_t = plus;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if( Control< Rule >::template match< A, M, Action, Control >( in, st... ) ) {
            while( Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
            }
            return true;
         }
         return false;
      }
   };

   template< typename Rule, typename... Rules >
   inline constexpr bool enable_control< plus< Rule, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 42 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/raise.hpp"
       
#line 1 "tao/pegtl/internal/raise.hpp"



#ifndef TAO_PEGTL_INTERNAL_RAISE_HPP
#define TAO_PEGTL_INTERNAL_RAISE_HPP

#include <stdexcept>
#line 17 "tao/pegtl/internal/raise.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   struct raise
   {
      using rule_t = raise;
      using subs_t = empty_list;

      template< apply_mode,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[noreturn]] static bool match( ParseInput& in, States&&... st )
      {
         Control< T >::raise( static_cast< const ParseInput& >( in ), st... );
      }
   };

   template< typename T >
   inline constexpr bool enable_control< raise< T > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 43 "tao/pegtl/internal/rules.hpp"



#line 1 "tao/pegtl/internal/rep.hpp"
       
#line 1 "tao/pegtl/internal/rep.hpp"



#ifndef TAO_PEGTL_INTERNAL_REP_HPP
#define TAO_PEGTL_INTERNAL_REP_HPP
#line 17 "tao/pegtl/internal/rep.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Cnt, typename... Rules >
   struct rep
      : rep< Cnt, seq< Rules... > >
   {};

   template< unsigned Cnt >
   struct rep< Cnt >
      : success
   {};

   template< typename Rule >
   struct rep< 0, Rule >
      : success
   {};

   template< unsigned Cnt, typename Rule >
   struct rep< Cnt, Rule >
   {
      using rule_t = rep;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();
         using m_t = decltype( m );

         for( unsigned i = 0; i != Cnt; ++i ) {
            if( !Control< Rule >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) ) {
               return false;
            }
         }
         return m( true );
      }
   };

   template< unsigned Cnt, typename... Rules >
   inline constexpr bool enable_control< rep< Cnt, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 47 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/rep_min.hpp"
       
#line 1 "tao/pegtl/internal/rep_min.hpp"



#ifndef TAO_PEGTL_INTERNAL_REP_MIN_HPP
#define TAO_PEGTL_INTERNAL_REP_MIN_HPP







namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Min, typename Rule, typename... Rules >
   using rep_min = seq< rep< Min, Rule, Rules... >, star< Rule, Rules... > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 48 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/rep_min_max.hpp"
       
#line 1 "tao/pegtl/internal/rep_min_max.hpp"



#ifndef TAO_PEGTL_INTERNAL_REP_MIN_MAX_HPP
#define TAO_PEGTL_INTERNAL_REP_MIN_MAX_HPP

#include <type_traits>
#line 20 "tao/pegtl/internal/rep_min_max.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Min, unsigned Max, typename... Rules >
   struct rep_min_max
      : rep_min_max< Min, Max, seq< Rules... > >
   {
      static_assert( Min <= Max );
   };

   template< unsigned Min, unsigned Max >
   struct rep_min_max< Min, Max >
      : failure
   {
      static_assert( Min <= Max );
   };

   template< typename Rule >
   struct rep_min_max< 0, 0, Rule >
      : not_at< Rule >
   {};

   template< unsigned Min, unsigned Max, typename Rule >
   struct rep_min_max< Min, Max, Rule >
   {
      using rule_t = rep_min_max;
      using subs_t = type_list< Rule >;

      static_assert( Min <= Max );

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();
         using m_t = decltype( m );

         for( unsigned i = 0; i != Min; ++i ) {
            if( !Control< Rule >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) ) {
               return false;
            }
         }
         for( unsigned i = Min; i != Max; ++i ) {
            if( !Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... ) ) {
               return m( true );
            }
         }
         return m( Control< not_at< Rule > >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) ); // NOTE that not_at<> will always rewind.
      }
   };

   template< unsigned Min, unsigned Max, typename... Rules >
   inline constexpr bool enable_control< rep_min_max< Min, Max, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 49 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/rep_opt.hpp"
       
#line 1 "tao/pegtl/internal/rep_opt.hpp"



#ifndef TAO_PEGTL_INTERNAL_REP_OPT_HPP
#define TAO_PEGTL_INTERNAL_REP_OPT_HPP
#line 17 "tao/pegtl/internal/rep_opt.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Max, typename... Rules >
   struct rep_opt
      : rep_opt< Max, seq< Rules... > >
   {};

   template< unsigned Max >
   struct rep_opt< Max >
      : success
   {};

   template< typename... Rules >
   struct rep_opt< 0, Rules... >
      : success
   {};

   template< unsigned Max, typename Rule >
   struct rep_opt< Max, Rule >
   {
      using rule_t = rep_opt;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         for( unsigned i = 0; ( i != Max ) && Control< Rule >::template match< A, rewind_mode::required, Action, Control >( in, st... ); ++i ) {
         }
         return true;
      }
   };

   template< unsigned Max, typename... Rules >
   inline constexpr bool enable_control< rep_opt< Max, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 50 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/require.hpp"
       
#line 1 "tao/pegtl/internal/require.hpp"



#ifndef TAO_PEGTL_INTERNAL_REQUIRE_HPP
#define TAO_PEGTL_INTERNAL_REQUIRE_HPP
#line 14 "tao/pegtl/internal/require.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< unsigned Amount >
   struct require;

   template<>
   struct require< 0 >
      : success
   {};

   template< unsigned Amount >
   struct require
   {
      using rule_t = require;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.size( 0 ) ) )
      {
         return in.size( Amount ) >= Amount;
      }
   };

   template< unsigned Amount >
   inline constexpr bool enable_control< require< Amount > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 51 "tao/pegtl/internal/rules.hpp"



#line 1 "tao/pegtl/internal/star_must.hpp"
       
#line 1 "tao/pegtl/internal/star_must.hpp"



#ifndef TAO_PEGTL_INTERNAL_STAR_MUST_HPP
#define TAO_PEGTL_INTERNAL_STAR_MUST_HPP






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Cond, typename... Rules >
   using star_must = star< if_must< false, Cond, Rules... > >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 55 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/state.hpp"
       
#line 1 "tao/pegtl/internal/state.hpp"



#ifndef TAO_PEGTL_INTERNAL_STATE_HPP
#define TAO_PEGTL_INTERNAL_STATE_HPP
#line 17 "tao/pegtl/internal/state.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename State, typename... Rules >
   struct state
      : state< State, seq< Rules... > >
   {};

   template< typename State >
   struct state< State >
      : success
   {};

   template< typename State, typename Rule >
   struct state< State, Rule >
   {
      using rule_t = state;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         State s( static_cast< const ParseInput& >( in ), st... );
         if( Control< Rule >::template match< A, M, Action, Control >( in, s ) ) {
            s.success( static_cast< const ParseInput& >( in ), st... );
            return true;
         }
         return false;
      }
   };

   template< typename State, typename... Rules >
   inline constexpr bool enable_control< state< State, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 56 "tao/pegtl/internal/rules.hpp"
#line 1 "tao/pegtl/internal/string.hpp"
       
#line 1 "tao/pegtl/internal/string.hpp"



#ifndef TAO_PEGTL_INTERNAL_STRING_HPP
#define TAO_PEGTL_INTERNAL_STRING_HPP

#include <cstring>
#include <utility>
#line 19 "tao/pegtl/internal/string.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   [[nodiscard]] inline bool unsafe_equals( const char* s, const std::initializer_list< char >& l ) noexcept
   {
      return std::memcmp( s, &*l.begin(), l.size() ) == 0;
   }

   template< char... Cs >
   struct string;

   template<>
   struct string<>
      : success
   {};

   template< char... Cs >
   struct string
   {
      using rule_t = string;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.size( 0 ) ) )
      {
         if( in.size( sizeof...( Cs ) ) >= sizeof...( Cs ) ) {
            if( unsafe_equals( in.current(), { Cs... } ) ) {
               bump_help< result_on_found::success, ParseInput, char, Cs... >( in, sizeof...( Cs ) );
               return true;
            }
         }
         return false;
      }
   };

   template< char... Cs >
   inline constexpr bool enable_control< string< Cs... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 57 "tao/pegtl/internal/rules.hpp"

#line 1 "tao/pegtl/internal/try_catch_type.hpp"
       
#line 1 "tao/pegtl/internal/try_catch_type.hpp"



#ifndef TAO_PEGTL_INTERNAL_TRY_CATCH_TYPE_HPP
#define TAO_PEGTL_INTERNAL_TRY_CATCH_TYPE_HPP

#include <type_traits>
#line 19 "tao/pegtl/internal/try_catch_type.hpp"
namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Exception, typename... Rules >
   struct try_catch_type
      : try_catch_type< Exception, seq< Rules... > >
   {};

   template< typename Exception >
   struct try_catch_type< Exception >
      : success
   {};

   template< typename Exception, typename Rule >
   struct try_catch_type< Exception, Rule >
   {
      using rule_t = try_catch_type;
      using subs_t = type_list< Rule >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         auto m = in.template mark< M >();
         using m_t = decltype( m );

         try {
            return m( Control< Rule >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) );
         }
         catch( const Exception& ) {
            return false;
         }
      }
   };

   template< typename Exception, typename... Rules >
   inline constexpr bool enable_control< try_catch_type< Exception, Rules... > > = false;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 59 "tao/pegtl/internal/rules.hpp"


#endif
#line 12 "tao/pegtl/ascii.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   inline namespace ascii
   {
      // clang-format off
      struct alnum : internal::ranges< internal::peek_char, 'a', 'z', 'A', 'Z', '0', '9' > {};
      struct alpha : internal::ranges< internal::peek_char, 'a', 'z', 'A', 'Z' > {};
      struct any : internal::any< internal::peek_char > {};
      struct blank : internal::one< internal::result_on_found::success, internal::peek_char, ' ', '\t' > {};
      struct digit : internal::range< internal::result_on_found::success, internal::peek_char, '0', '9' > {};
      struct ellipsis : internal::string< '.', '.', '.' > {};
      template< char... Cs > struct forty_two : internal::rep< 42, internal::one< internal::result_on_found::success, internal::peek_char, Cs... > > {};
      struct identifier_first : internal::identifier_first {};
      struct identifier_other : internal::identifier_other {};
      struct identifier : internal::identifier {};
      template< char... Cs > struct istring : internal::istring< Cs... > {};
      template< char... Cs > struct keyword : internal::seq< internal::string< Cs... >, internal::not_at< internal::identifier_other > > { static_assert( sizeof...( Cs ) > 0 ); };
      struct lower : internal::range< internal::result_on_found::success, internal::peek_char, 'a', 'z' > {};
      template< char... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_char, Cs... > {};
      template< char Lo, char Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_char, Lo, Hi > {};
      struct nul : internal::one< internal::result_on_found::success, internal::peek_char, char( 0 ) > {};
      template< char... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_char, Cs... > {};
      struct print : internal::range< internal::result_on_found::success, internal::peek_char, char( 32 ), char( 126 ) > {};
      template< char Lo, char Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_char, Lo, Hi > {};
      template< char... Cs > struct ranges : internal::ranges< internal::peek_char, Cs... > {};
      struct seven : internal::range< internal::result_on_found::success, internal::peek_char, char( 0 ), char( 127 ) > {};
      struct shebang : internal::seq< internal::string< '#', '!' >, internal::until< internal::eolf > > {};
      struct space : internal::one< internal::result_on_found::success, internal::peek_char, ' ', '\n', '\r', '\t', '\v', '\f' > {};
      template< char... Cs > struct string : internal::string< Cs... > {};
      template< char C > struct three : internal::string< C, C, C > {};
      template< char C > struct two : internal::string< C, C > {};
      struct upper : internal::range< internal::result_on_found::success, internal::peek_char, 'A', 'Z' > {};
      struct xdigit : internal::ranges< internal::peek_char, '0', '9', 'a', 'f', 'A', 'F' > {};
      // clang-format on

   } // namespace ascii

} // namespace TAO_PEGTL_NAMESPACE

#line 1 "tao/pegtl/internal/pegtl_string.hpp"
       
#line 1 "tao/pegtl/internal/pegtl_string.hpp"



#ifndef TAO_PEGTL_INTERNAL_PEGTL_STRING_HPP
#define TAO_PEGTL_INTERNAL_PEGTL_STRING_HPP

#include <cstddef>
#include <type_traits>




namespace TAO_PEGTL_NAMESPACE::internal
{
   // Inspired by https://github.com/irrequietus/typestring
   // Rewritten and reduced to what is needed for the PEGTL
   // and to work with Visual Studio 2015.

   template< typename, typename, typename, typename, typename, typename, typename, typename >
   struct string_join;

   template< template< char... > class S, char... C0s, char... C1s, char... C2s, char... C3s, char... C4s, char... C5s, char... C6s, char... C7s >
   struct string_join< S< C0s... >, S< C1s... >, S< C2s... >, S< C3s... >, S< C4s... >, S< C5s... >, S< C6s... >, S< C7s... > >
   {
      using type = S< C0s..., C1s..., C2s..., C3s..., C4s..., C5s..., C6s..., C7s... >;
   };

   template< template< char... > class S, char, bool >
   struct string_at
   {
      using type = S<>;
   };

   template< template< char... > class S, char C >
   struct string_at< S, C, true >
   {
      using type = S< C >;
   };

   template< typename T, std::size_t S >
   struct string_max_length
   {
      static_assert( S <= 512, "String longer than 512 (excluding terminating \\0)!" );
      using type = T;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#define TAO_PEGTL_INTERNAL_EMPTY()
#define TAO_PEGTL_INTERNAL_DEFER( X ) X TAO_PEGTL_INTERNAL_EMPTY()
#define TAO_PEGTL_INTERNAL_EXPAND( ... ) __VA_ARGS__

#define TAO_PEGTL_INTERNAL_STRING_AT( S, x, n )    TAO_PEGTL_NAMESPACE::internal::string_at< S, ( 0##n < ( sizeof( x ) / sizeof( char ) ) ) ? ( x )[ 0##n ] : 0, ( 0##n < ( sizeof( x ) / sizeof( char ) ) - 1 ) >::type


#define TAO_PEGTL_INTERNAL_JOIN_8( M, S, x, n )                                                TAO_PEGTL_NAMESPACE::internal::string_join< TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##0 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##1 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##2 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##3 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##4 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##5 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##6 ),                                                TAO_PEGTL_INTERNAL_DEFER( M )( S, x, n##7 ) >::type
#line 66 "tao/pegtl/internal/pegtl_string.hpp"
#define TAO_PEGTL_INTERNAL_STRING_8( S, x, n )    TAO_PEGTL_INTERNAL_JOIN_8( TAO_PEGTL_INTERNAL_STRING_AT, S, x, n )


#define TAO_PEGTL_INTERNAL_STRING_64( S, x, n )    TAO_PEGTL_INTERNAL_JOIN_8( TAO_PEGTL_INTERNAL_STRING_8, S, x, n )


#define TAO_PEGTL_INTERNAL_STRING_512( S, x, n )    TAO_PEGTL_INTERNAL_JOIN_8( TAO_PEGTL_INTERNAL_STRING_64, S, x, n )


#define TAO_PEGTL_INTERNAL_STRING( S, x )    TAO_PEGTL_INTERNAL_EXPAND(                   TAO_PEGTL_INTERNAL_EXPAND(                   TAO_PEGTL_INTERNAL_EXPAND(                   TAO_PEGTL_NAMESPACE::internal::string_max_length< TAO_PEGTL_INTERNAL_STRING_512( S, x, ), sizeof( x ) - 1 >::type ) ) )





#define TAO_PEGTL_STRING( x )    TAO_PEGTL_INTERNAL_STRING( TAO_PEGTL_NAMESPACE::ascii::string, x )


#define TAO_PEGTL_ISTRING( x )    TAO_PEGTL_INTERNAL_STRING( TAO_PEGTL_NAMESPACE::ascii::istring, x )


#define TAO_PEGTL_KEYWORD( x )    TAO_PEGTL_INTERNAL_STRING( TAO_PEGTL_NAMESPACE::ascii::keyword, x )


#endif
#line 53 "tao/pegtl/ascii.hpp"

#endif
#line 12 "tao/pegtl.hpp"
#line 1 "tao/pegtl/rules.hpp"
       
#line 1 "tao/pegtl/rules.hpp"



#ifndef TAO_PEGTL_RULES_HPP
#define TAO_PEGTL_RULES_HPP






namespace TAO_PEGTL_NAMESPACE
{
   // clang-format off
   template< template< typename... > class Action, typename... Rules > struct action : internal::action< Action, Rules... > {};
   template< typename... Actions > struct apply : internal::apply< Actions... > {};
   template< typename... Actions > struct apply0 : internal::apply0< Actions... > {};
   template< typename... Rules > struct at : internal::at< Rules... > {};
   struct bof : internal::bof {};
   struct bol : internal::bol {};
   template< unsigned Num > struct bytes : internal::bytes< Num > {};
   template< template< typename... > class Control, typename... Rules > struct control : internal::control< Control, Rules... > {};
   template< typename... Rules > struct disable : internal::disable< Rules... > {};
   struct discard : internal::discard {};
   template< typename... Rules > struct enable : internal::enable< Rules... > {};
   struct eof : internal::eof {};
   struct eolf : internal::eolf {};
   struct failure : internal::failure {};
   template< typename Rule, typename... Actions > struct if_apply : internal::if_apply< Rule, Actions... > {};
   template< typename Cond, typename... Thens > struct if_must : internal::if_must< false, Cond, Thens... > {};
   template< typename Cond, typename Then, typename Else > struct if_must_else : internal::if_must_else< Cond, Then, Else > {};
   template< typename Cond, typename Then, typename Else > struct if_then_else : internal::if_then_else< Cond, Then, Else > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list : internal::list< Rule, internal::pad< Sep, Pad > > {};
   template< typename Rule, typename Sep > struct list< Rule, Sep, void > : internal::list< Rule, Sep > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list_must : internal::list_must< Rule, internal::pad< Sep, Pad > > {};
   template< typename Rule, typename Sep > struct list_must< Rule, Sep, void > : internal::list_must< Rule, Sep > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list_tail : internal::list_tail_pad< Rule, Sep, Pad > {};
   template< typename Rule, typename Sep > struct list_tail< Rule, Sep, void > : internal::list_tail< Rule, Sep > {};
   template< typename M, typename S > struct minus : internal::minus< M, S > {};
   template< typename... Rules > struct must : internal::must< Rules... > {};
   template< typename... Rules > struct not_at : internal::not_at< Rules... > {};
   template< typename... Rules > struct opt : internal::opt< Rules... > {};
   template< typename Cond, typename... Rules > struct opt_must : internal::if_must< true, Cond, Rules... > {};
   template< typename Rule, typename Pad1, typename Pad2 = Pad1 > struct pad : internal::pad< Rule, Pad1, Pad2 > {};
   template< typename Rule, typename Pad > struct pad_opt : internal::pad_opt< Rule, Pad > {};
   template< typename Rule, typename... Rules > struct plus : internal::plus< Rule, Rules... > {};
   template< typename Exception > struct raise : internal::raise< Exception > {};
   template< typename Head, typename... Rules > struct rematch : internal::rematch< Head, Rules... > {};
   template< unsigned Num, typename... Rules > struct rep : internal::rep< Num, Rules... > {};
   template< unsigned Max, typename... Rules > struct rep_max : internal::rep_min_max< 0, Max, Rules... > {};
   template< unsigned Min, typename Rule, typename... Rules > struct rep_min : internal::rep_min< Min, Rule, Rules... > {};
   template< unsigned Min, unsigned Max, typename... Rules > struct rep_min_max : internal::rep_min_max< Min, Max, Rules... > {};
   template< unsigned Max, typename... Rules > struct rep_opt : internal::rep_opt< Max, Rules... > {};
   template< unsigned Amount > struct require : internal::require< Amount > {};
   template< typename... Rules > struct seq : internal::seq< Rules... > {};
   template< typename... Rules > struct sor : internal::sor< Rules... > {};
   template< typename Rule, typename... Rules > struct star : internal::star< Rule, Rules... > {};
   template< typename Cond, typename... Rules > struct star_must : internal::star_must< Cond, Rules... > {};
   template< typename State, typename... Rules > struct state : internal::state< State, Rules... > {};
   struct success : internal::success {};
   template< typename... Rules > struct try_catch : internal::try_catch_type< parse_error, Rules... > {};
   template< typename Exception, typename... Rules > struct try_catch_type : internal::seq< internal::try_catch_type< Exception, Rules... > > {};
   template< typename Cond, typename... Rules > struct until : internal::until< Cond, Rules... > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 13 "tao/pegtl.hpp"
#line 1 "tao/pegtl/utf8.hpp"
       
#line 1 "tao/pegtl/utf8.hpp"



#ifndef TAO_PEGTL_UTF8_HPP
#define TAO_PEGTL_UTF8_HPP



#line 1 "tao/pegtl/internal/peek_utf8.hpp"
       
#line 1 "tao/pegtl/internal/peek_utf8.hpp"



#ifndef TAO_PEGTL_INTERNAL_PEEK_UTF8_HPP
#define TAO_PEGTL_INTERNAL_PEEK_UTF8_HPP





namespace TAO_PEGTL_NAMESPACE::internal
{
   struct peek_utf8
   {
      using data_t = char32_t;
      using pair_t = input_pair< char32_t >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( in.empty() ) {
            return { 0, 0 };
         }
         const char32_t c0 = in.peek_uint8();
         if( ( c0 & 0x80 ) == 0 ) {
            return { c0, 1 };
         }
         return peek_impl( in, c0 );
      }

   private:
      template< typename ParseInput >
      [[nodiscard]] static pair_t peek_impl( ParseInput& in, char32_t c0 ) noexcept( noexcept( in.size( 4 ) ) )
      {
         if( ( c0 & 0xE0 ) == 0xC0 ) {
            if( in.size( 2 ) >= 2 ) {
               const char32_t c1 = in.peek_uint8( 1 );
               if( ( c1 & 0xC0 ) == 0x80 ) {
                  c0 &= 0x1F;
                  c0 <<= 6;
                  c0 |= ( c1 & 0x3F );
                  if( c0 >= 0x80 ) {
                     return { c0, 2 };
                  }
               }
            }
         }
         else if( ( c0 & 0xF0 ) == 0xE0 ) {
            if( in.size( 3 ) >= 3 ) {
               const char32_t c1 = in.peek_uint8( 1 );
               const char32_t c2 = in.peek_uint8( 2 );
               if( ( ( c1 & 0xC0 ) == 0x80 ) && ( ( c2 & 0xC0 ) == 0x80 ) ) {
                  c0 &= 0x0F;
                  c0 <<= 6;
                  c0 |= ( c1 & 0x3F );
                  c0 <<= 6;
                  c0 |= ( c2 & 0x3F );
                  if( c0 >= 0x800 && !( c0 >= 0xD800 && c0 <= 0xDFFF ) ) {
                     return { c0, 3 };
                  }
               }
            }
         }
         else if( ( c0 & 0xF8 ) == 0xF0 ) {
            if( in.size( 4 ) >= 4 ) {
               const char32_t c1 = in.peek_uint8( 1 );
               const char32_t c2 = in.peek_uint8( 2 );
               const char32_t c3 = in.peek_uint8( 3 );
               if( ( ( c1 & 0xC0 ) == 0x80 ) && ( ( c2 & 0xC0 ) == 0x80 ) && ( ( c3 & 0xC0 ) == 0x80 ) ) {
                  c0 &= 0x07;
                  c0 <<= 6;
                  c0 |= ( c1 & 0x3F );
                  c0 <<= 6;
                  c0 |= ( c2 & 0x3F );
                  c0 <<= 6;
                  c0 |= ( c3 & 0x3F );
                  if( c0 >= 0x10000 && c0 <= 0x10FFFF ) {
                     return { c0, 4 };
                  }
               }
            }
         }
         return { 0, 0 };
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 10 "tao/pegtl/utf8.hpp"



namespace TAO_PEGTL_NAMESPACE::utf8
{
   // clang-format off
   struct any : internal::any< internal::peek_utf8 > {};
   struct bom : internal::one< internal::result_on_found::success, internal::peek_utf8, 0xfeff > {};
   template< char32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_utf8, Cs... > {};
   template< char32_t Lo, char32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_utf8, Lo, Hi > {};
   template< char32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_utf8, Cs... > {};
   template< char32_t Lo, char32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_utf8, Lo, Hi > {};
   template< char32_t... Cs > struct ranges : internal::ranges< internal::peek_utf8, Cs... > {};
   template< char32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_utf8, Cs >... > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::utf8

#endif
#line 14 "tao/pegtl.hpp"

#line 1 "tao/pegtl/argv_input.hpp"
       
#line 1 "tao/pegtl/argv_input.hpp"



#ifndef TAO_PEGTL_ARGV_INPUT_HPP
#define TAO_PEGTL_ARGV_INPUT_HPP

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>






namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      [[nodiscard]] inline std::string make_argv_source( const std::size_t argn )
      {
         std::ostringstream os;
         os << "argv[" << argn << ']';
         return os.str();
      }

   } // namespace internal

   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
   struct argv_input
      : memory_input< P, Eol >
   {
      template< typename T >
      argv_input( char** argv, const std::size_t argn, T&& in_source )
         : memory_input< P, Eol >( static_cast< const char* >( argv[ argn ] ), std::forward< T >( in_source ) )
      {}

      argv_input( char** argv, const std::size_t argn )
         : argv_input( argv, argn, internal::make_argv_source( argn ) )
      {}
   };

   template< typename... Ts >
   argv_input( Ts&&... ) -> argv_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 16 "tao/pegtl.hpp"
#line 1 "tao/pegtl/buffer_input.hpp"
       
#line 1 "tao/pegtl/buffer_input.hpp"



#ifndef TAO_PEGTL_BUFFER_INPUT_HPP
#define TAO_PEGTL_BUFFER_INPUT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#line 27 "tao/pegtl/buffer_input.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   template< typename Reader, typename Eol = eol::lf_crlf, typename Source = std::string, std::size_t Chunk = 64 >
   class buffer_input
   {
   public:
      using reader_t = Reader;

      using eol_t = Eol;
      using source_t = Source;

      using iterator_t = internal::iterator;

      using action_t = internal::action_input< buffer_input >;

      static constexpr std::size_t chunk_size = Chunk;
      static constexpr tracking_mode tracking_mode_v = tracking_mode::eager;

      template< typename T, typename... As >
      buffer_input( T&& in_source, const std::size_t maximum, As&&... as )
         : m_reader( std::forward< As >( as )... ),
           m_maximum( maximum + Chunk ),
           m_buffer( new char[ maximum + Chunk ] ),
           m_current( m_buffer.get() ),
           m_end( m_buffer.get() ),
           m_source( std::forward< T >( in_source ) )
      {
         static_assert( Chunk, "zero chunk size not implemented" );
         assert( m_maximum > maximum ); // Catches overflow; change to >= when zero chunk size is implemented.
      }

      buffer_input( const buffer_input& ) = delete;
      buffer_input( buffer_input&& ) = delete;

      ~buffer_input() = default;

      void operator=( const buffer_input& ) = delete;
      void operator=( buffer_input&& ) = delete;

      [[nodiscard]] bool empty()
      {
         require( 1 );
         return m_current.data == m_end;
      }

      [[nodiscard]] std::size_t size( const std::size_t amount )
      {
         require( amount );
         return buffer_occupied();
      }

      [[nodiscard]] const char* current() const noexcept
      {
         return m_current.data;
      }

      [[nodiscard]] const char* end( const std::size_t amount )
      {
         require( amount );
         return m_end;
      }

      [[nodiscard]] std::size_t byte() const noexcept
      {
         return m_current.byte;
      }

      [[nodiscard]] std::size_t line() const noexcept
      {
         return m_current.line;
      }

      [[nodiscard]] std::size_t column() const noexcept
      {
         return m_current.column;
      }

      [[nodiscard]] const Source& source() const noexcept
      {
         return m_source;
      }

      [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept
      {
         return m_current.data[ offset ];
      }

      [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept
      {
         return static_cast< std::uint8_t >( peek_char( offset ) );
      }

      void bump( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump( m_current, in_count, Eol::ch );
      }

      void bump_in_this_line( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump_in_this_line( m_current, in_count );
      }

      void bump_to_next_line( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump_to_next_line( m_current, in_count );
      }

      void discard() noexcept
      {
         if( m_current.data > m_buffer.get() + Chunk ) {
            const auto s = m_end - m_current.data;
            std::memmove( m_buffer.get(), m_current.data, s );
            m_current.data = m_buffer.get();
            m_end = m_buffer.get() + s;
         }
      }

      void require( const std::size_t amount )
      {
         if( m_current.data + amount <= m_end ) {
            return;
         }
         if( m_current.data + amount > m_buffer.get() + m_maximum ) {
            throw std::overflow_error( "require beyond end of buffer" );
         }
         if( const auto r = m_reader( m_end, ( std::min )( buffer_free_after_end(), ( std::max )( amount - buffer_occupied(), Chunk ) ) ) ) {
            m_end += r;
         }
      }

      template< rewind_mode M >
      [[nodiscard]] internal::marker< iterator_t, M > mark() noexcept
      {
         return internal::marker< iterator_t, M >( m_current );
      }

      [[nodiscard]] TAO_PEGTL_NAMESPACE::position position( const iterator_t& it ) const
      {
         return TAO_PEGTL_NAMESPACE::position( it, m_source );
      }

      [[nodiscard]] TAO_PEGTL_NAMESPACE::position position() const
      {
         return position( m_current );
      }

      [[nodiscard]] const iterator_t& iterator() const noexcept
      {
         return m_current;
      }

      [[nodiscard]] std::size_t buffer_capacity() const noexcept
      {
         return m_maximum;
      }

      [[nodiscard]] std::size_t buffer_occupied() const noexcept
      {
         assert( m_end >= m_current.data );
         return std::size_t( m_end - m_current.data );
      }

      [[nodiscard]] std::size_t buffer_free_before_current() const noexcept
      {
         assert( m_current.data >= m_buffer.get() );
         return std::size_t( m_current.data - m_buffer.get() );
      }

      [[nodiscard]] std::size_t buffer_free_after_end() const noexcept
      {
         assert( m_buffer.get() + m_maximum >= m_end );
         return std::size_t( m_buffer.get() + m_maximum - m_end );
      }

   private:
      Reader m_reader;
      std::size_t m_maximum;
      std::unique_ptr< char[] > m_buffer;
      iterator_t m_current;
      char* m_end;
      const Source m_source;
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 17 "tao/pegtl.hpp"
#line 1 "tao/pegtl/cstream_input.hpp"
       
#line 1 "tao/pegtl/cstream_input.hpp"



#ifndef TAO_PEGTL_CSTREAM_INPUT_HPP
#define TAO_PEGTL_CSTREAM_INPUT_HPP

#include <cstdio>





#line 1 "tao/pegtl/internal/cstream_reader.hpp"
       
#line 1 "tao/pegtl/internal/cstream_reader.hpp"



#ifndef TAO_PEGTL_INTERNAL_CSTREAM_READER_HPP
#define TAO_PEGTL_INTERNAL_CSTREAM_READER_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>

#include <system_error>



namespace TAO_PEGTL_NAMESPACE::internal
{
   struct cstream_reader
   {
      explicit cstream_reader( std::FILE* s ) noexcept
         : m_cstream( s )
      {
         assert( m_cstream != nullptr );
      }

      [[nodiscard]] std::size_t operator()( char* buffer, const std::size_t length ) const
      {
         if( const auto r = std::fread( buffer, 1, length, m_cstream ) ) {
            return r;
         }
         if( std::feof( m_cstream ) != 0 ) {
            return 0;
         }

         // Please contact us if you know how to provoke the following exception.
         // The example on cppreference.com doesn't work, at least not on macOS.

         // LCOV_EXCL_START
         const auto ec = std::ferror( m_cstream );
         assert( ec != 0 );
         throw std::system_error( ec, std::system_category(), "fread() failed" );
         // LCOV_EXCL_STOP
      }

      std::FILE* m_cstream;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/cstream_input.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Eol = eol::lf_crlf, std::size_t Chunk = 64 >
   struct cstream_input
      : buffer_input< internal::cstream_reader, Eol, std::string, Chunk >
   {
      template< typename T >
      cstream_input( std::FILE* in_stream, const std::size_t in_maximum, T&& in_source )
         : buffer_input< internal::cstream_reader, Eol, std::string, Chunk >( std::forward< T >( in_source ), in_maximum, in_stream )
      {}
   };

   template< typename... Ts >
   cstream_input( Ts&&... ) -> cstream_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 18 "tao/pegtl.hpp"
#line 1 "tao/pegtl/istream_input.hpp"
       
#line 1 "tao/pegtl/istream_input.hpp"



#ifndef TAO_PEGTL_ISTREAM_INPUT_HPP
#define TAO_PEGTL_ISTREAM_INPUT_HPP

#include <istream>





#line 1 "tao/pegtl/internal/istream_reader.hpp"
       
#line 1 "tao/pegtl/internal/istream_reader.hpp"



#ifndef TAO_PEGTL_INTERNAL_ISTREAM_READER_HPP
#define TAO_PEGTL_INTERNAL_ISTREAM_READER_HPP

#include <istream>
#include <system_error>



namespace TAO_PEGTL_NAMESPACE::internal
{
   struct istream_reader
   {
      explicit istream_reader( std::istream& s ) noexcept
         : m_istream( s )
      {}

      [[nodiscard]] std::size_t operator()( char* buffer, const std::size_t length )
      {
         m_istream.read( buffer, std::streamsize( length ) );

         if( const auto r = m_istream.gcount() ) {
            return std::size_t( r );
         }
         if( m_istream.eof() ) {
            return 0;
         }
         const auto ec = errno;
         throw std::system_error( ec, std::system_category(), "std::istream::read() failed" );
      }

      std::istream& m_istream;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/istream_input.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Eol = eol::lf_crlf, std::size_t Chunk = 64 >
   struct istream_input
      : buffer_input< internal::istream_reader, Eol, std::string, Chunk >
   {
      template< typename T >
      istream_input( std::istream& in_stream, const std::size_t in_maximum, T&& in_source )
         : buffer_input< internal::istream_reader, Eol, std::string, Chunk >( std::forward< T >( in_source ), in_maximum, in_stream )
      {}
   };

   template< typename... Ts >
   istream_input( Ts&&... ) -> istream_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 19 "tao/pegtl.hpp"

#line 1 "tao/pegtl/read_input.hpp"
       
#line 1 "tao/pegtl/read_input.hpp"



#ifndef TAO_PEGTL_READ_INPUT_HPP
#define TAO_PEGTL_READ_INPUT_HPP

#include <filesystem>
#include <string>



#line 1 "tao/pegtl/string_input.hpp"
       
#line 1 "tao/pegtl/string_input.hpp"



#ifndef TAO_PEGTL_STRING_INPUT_HPP
#define TAO_PEGTL_STRING_INPUT_HPP

#include <string>
#include <utility>






namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      struct string_holder
      {
         const std::string data;

         template< typename T >
         explicit string_holder( T&& in_data )
            : data( std::forward< T >( in_data ) )
         {}

         string_holder( const string_holder& ) = delete;
         string_holder( string_holder&& ) = delete;

         ~string_holder() = default;

         void operator=( const string_holder& ) = delete;
         void operator=( string_holder&& ) = delete;
      };

   } // namespace internal

   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf, typename Source = std::string >
   struct string_input
      : private internal::string_holder,
        public memory_input< P, Eol, Source >
   {
      template< typename V, typename T, typename... Ts >
      explicit string_input( V&& in_data, T&& in_source, Ts&&... ts )
         : internal::string_holder( std::forward< V >( in_data ) ),
           memory_input< P, Eol, Source >( data.data(), data.size(), std::forward< T >( in_source ), std::forward< Ts >( ts )... )
      {}

      string_input( const string_input& ) = delete;
      string_input( string_input&& ) = delete;

      ~string_input() = default;

      void operator=( const string_input& ) = delete;
      void operator=( string_input&& ) = delete;
   };

   template< typename... Ts >
   explicit string_input( Ts&&... ) -> string_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 13 "tao/pegtl/read_input.hpp"


#line 1 "tao/pegtl/internal/file_reader.hpp"
       
#line 1 "tao/pegtl/internal/file_reader.hpp"



#ifndef TAO_PEGTL_INTERNAL_FILE_READER_HPP
#define TAO_PEGTL_INTERNAL_FILE_READER_HPP

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   [[nodiscard]] inline std::FILE* file_open( const std::filesystem::path& path )
   {
      errno = 0;
#if defined( _MSC_VER )
      std::FILE* file;
      if( ::_wfopen_s( &file, path.c_str(), L"rb" ) == 0 ) {
         return file;
      }
      const std::error_code ec( errno, std::system_category() );
      throw std::filesystem::filesystem_error( "_wfopen_s() failed", path, ec );
#else
#if defined( __MINGW32__ )
      if( auto* file = std::fopen( path.c_str(), "rb" ) )
#else
      if( auto* file = std::fopen( path.c_str(), "rbe" ) )
#endif
      {
         return file;
      }
      const std::error_code ec( errno, std::system_category() );
      throw std::filesystem::filesystem_error( "std::fopen() failed", path, ec );
#endif
   }

   struct file_close
   {
      void operator()( FILE* f ) const noexcept
      {
         std::fclose( f );
      }
   };

   class file_reader
   {
   public:
      explicit file_reader( const std::filesystem::path& path )
         : file_reader( file_open( path ), path )
      {}

      file_reader( FILE* file, const std::filesystem::path& path ) // NOLINT(modernize-pass-by-value)
         : m_path( path ),
           m_file( file )
      {}

      file_reader( const file_reader& ) = delete;
      file_reader( file_reader&& ) = delete;

      ~file_reader() = default;

      void operator=( const file_reader& ) = delete;
      void operator=( file_reader&& ) = delete;

      [[nodiscard]] std::size_t size() const
      {
         errno = 0;
         if( std::fseek( m_file.get(), 0, SEEK_END ) != 0 ) {
            // LCOV_EXCL_START
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fseek() failed [SEEK_END]", m_path, ec );
            // LCOV_EXCL_STOP
         }
         errno = 0;
         const auto s = std::ftell( m_file.get() );
         if( s < 0 ) {
            // LCOV_EXCL_START
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::ftell() failed", m_path, ec );
            // LCOV_EXCL_STOP
         }
         errno = 0;
         if( std::fseek( m_file.get(), 0, SEEK_SET ) != 0 ) {
            // LCOV_EXCL_START
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fseek() failed [SEEK_SET]", m_path, ec );
            // LCOV_EXCL_STOP
         }
         return std::size_t( s );
      }

      [[nodiscard]] std::string read() const
      {
         std::string nrv;
         nrv.resize( size() );
         errno = 0;
         if( !nrv.empty() && ( std::fread( &nrv[ 0 ], nrv.size(), 1, m_file.get() ) != 1 ) ) {
            // LCOV_EXCL_START
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fread() failed", m_path, ec );
            // LCOV_EXCL_STOP
         }
         return nrv;
      }

   private:
      const std::filesystem::path m_path;
      const std::unique_ptr< std::FILE, file_close > m_file;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 16 "tao/pegtl/read_input.hpp"
#line 1 "tao/pegtl/internal/path_to_string.hpp"
       
#line 1 "tao/pegtl/internal/path_to_string.hpp"



#ifndef TAO_PEGTL_INTERNAL_PATH_TO_STRING_HPP
#define TAO_PEGTL_INTERNAL_PATH_TO_STRING_HPP

#include <filesystem>
#include <string>



namespace TAO_PEGTL_NAMESPACE::internal
{
   [[nodiscard]] inline std::string path_to_string( const std::filesystem::path& path )
   {
#if defined( __cpp_char8_t )
      const auto s = path.u8string();
      return { reinterpret_cast< const char* >( s.data() ), s.size() };
#else
      return path.u8string();
#endif
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 17 "tao/pegtl/read_input.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
   struct read_input
      : string_input< P, Eol >
   {
      read_input( const std::filesystem::path& path, const std::string& source )
         : string_input< P, Eol >( internal::file_reader( path ).read(), source )
      {}

      explicit read_input( const std::filesystem::path& path )
         : read_input( path, internal::path_to_string( path ) )
      {}

      read_input( FILE* file, const std::filesystem::path& path, const std::string& source )
         : string_input< P, Eol >( internal::file_reader( file, path ).read(), source )
      {}

      read_input( FILE* file, const std::filesystem::path& path )
         : read_input( file, path, internal::path_to_string( path ) )
      {}

      read_input( const read_input& ) = delete;
      read_input( read_input&& ) = delete;

      ~read_input() = default;

      void operator=( const read_input& ) = delete;
      void operator=( read_input&& ) = delete;
   };

   template< typename... Ts >
   explicit read_input( Ts&&... ) -> read_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 21 "tao/pegtl.hpp"


// This has to be included *after* the above inputs,
// otherwise the amalgamated header will not work!
#line 1 "tao/pegtl/file_input.hpp"
       
#line 1 "tao/pegtl/file_input.hpp"



#ifndef TAO_PEGTL_FILE_INPUT_HPP
#define TAO_PEGTL_FILE_INPUT_HPP





#if defined( __unix__ ) || ( defined( __APPLE__ ) && defined( __MACH__ ) )
#include <unistd.h>  // Required for _POSIX_MAPPED_FILES
#endif

#if defined( _POSIX_MAPPED_FILES ) || defined( _WIN32 )
#line 1 "tao/pegtl/mmap_input.hpp"
       
#line 1 "tao/pegtl/mmap_input.hpp"



#ifndef TAO_PEGTL_MMAP_INPUT_HPP
#define TAO_PEGTL_MMAP_INPUT_HPP

#include <filesystem>
#include <string>
#line 17 "tao/pegtl/mmap_input.hpp"
#if defined( __unix__ ) || ( defined( __APPLE__ ) && defined( __MACH__ ) )
#include <unistd.h>  // Required for _POSIX_MAPPED_FILES
#endif

#if defined( _POSIX_MAPPED_FILES )
#line 1 "tao/pegtl/internal/file_mapper_posix.hpp"
       
#line 1 "tao/pegtl/internal/file_mapper_posix.hpp"



#ifndef TAO_PEGTL_INTERNAL_FILE_MAPPER_POSIX_HPP
#define TAO_PEGTL_INTERNAL_FILE_MAPPER_POSIX_HPP

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   struct file_opener
   {
      explicit file_opener( const std::filesystem::path& path ) // NOLINT(modernize-pass-by-value)
         : m_path( path ),
           m_fd( open() )
      {}

      file_opener( const file_opener& ) = delete;
      file_opener( file_opener&& ) = delete;

      ~file_opener()
      {
         ::close( m_fd );
      }

      void operator=( const file_opener& ) = delete;
      void operator=( file_opener&& ) = delete;

      [[nodiscard]] std::size_t size() const
      {
         struct stat st;
         errno = 0;
         if( ::fstat( m_fd, &st ) < 0 ) {
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "fstat() failed", m_path, ec );
         }
         return std::size_t( st.st_size );
      }

      const std::filesystem::path m_path;
      const int m_fd;

   private:
      [[nodiscard]] int open() const
      {
         errno = 0;
         const int fd = ::open( m_path.c_str(),
                                O_RDONLY
#if defined( O_CLOEXEC )
                                   | O_CLOEXEC
#endif
         );
         if( fd >= 0 ) {
            return fd;
         }
         const std::error_code ec( errno, std::system_category() );
         throw std::filesystem::filesystem_error( "open() failed", m_path, ec );
      }
   };

   class file_mapper
   {
   public:
      explicit file_mapper( const std::filesystem::path& path )
         : file_mapper( file_opener( path ) )
      {}

      explicit file_mapper( const file_opener& reader )
         : m_size( reader.size() ),
           m_data( static_cast< const char* >( ::mmap( nullptr, m_size, PROT_READ, MAP_PRIVATE, reader.m_fd, 0 ) ) )
      {
         if( ( m_size != 0 ) && ( intptr_t( m_data ) == -1 ) ) {
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "mmap() failed", reader.m_path, ec );
         }
      }

      file_mapper( const file_mapper& ) = delete;
      file_mapper( file_mapper&& ) = delete;

      ~file_mapper()
      {
         // Legacy C interface requires pointer-to-mutable but does not write through the pointer.
         ::munmap( const_cast< char* >( m_data ), m_size );
      }

      void operator=( const file_mapper& ) = delete;
      void operator=( file_mapper&& ) = delete;

      [[nodiscard]] bool empty() const noexcept
      {
         return m_size == 0;
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return m_size;
      }

      using iterator = const char*;
      using const_iterator = const char*;

      [[nodiscard]] iterator data() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator begin() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator end() const noexcept
      {
         return m_data + m_size;
      }

   private:
      const std::size_t m_size;
      const char* const m_data;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 23 "tao/pegtl/mmap_input.hpp"
#elif defined( _WIN32 )
#line 1 "tao/pegtl/internal/file_mapper_win32.hpp"
       
#line 1 "tao/pegtl/internal/file_mapper_win32.hpp"



#ifndef TAO_PEGTL_INTERNAL_FILE_MAPPER_WIN32_HPP
#define TAO_PEGTL_INTERNAL_FILE_MAPPER_WIN32_HPP

#if !defined( NOMINMAX )
#define NOMINMAX
#define TAO_PEGTL_NOMINMAX_WAS_DEFINED
#endif

#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#define TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED
#endif

#include <windows.h>

#if defined( TAO_PEGTL_NOMINMAX_WAS_DEFINED )
#undef NOMINMAX
#undef TAO_PEGTL_NOMINMAX_WAS_DEFINED
#endif

#if defined( TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED )
#undef WIN32_LEAN_AND_MEAN
#undef TAO_PEGTL_WIN32_LEAN_AND_MEAN_WAS_DEFINED
#endif

#include <filesystem>



namespace TAO_PEGTL_NAMESPACE::internal
{
   struct win32_file_opener
   {
      explicit win32_file_opener( const std::filesystem::path& path )
         : m_path( path ),
           m_handle( open() )
      {}

      win32_file_opener( const win32_file_opener& ) = delete;
      win32_file_opener( win32_file_opener&& ) = delete;

      ~win32_file_opener()
      {
         ::CloseHandle( m_handle );
      }

      void operator=( const win32_file_opener& ) = delete;
      void operator=( win32_file_opener&& ) = delete;

      [[nodiscard]] std::size_t size() const
      {
         LARGE_INTEGER size;
         if( !::GetFileSizeEx( m_handle, &size ) ) {
            const std::error_code ec( ::GetLastError(), std::system_category() );
            throw std::filesystem::filesystem_error( "GetFileSizeEx() failed", m_path, ec );
         }
         return std::size_t( size.QuadPart );
      }

      const std::filesystem::path m_path;
      const HANDLE m_handle;

   private:
      [[nodiscard]] HANDLE open() const
      {
         SetLastError( 0 );
#if( _WIN32_WINNT >= 0x0602 )
         const HANDLE handle = ::CreateFile2( m_path.c_str(),
                                              GENERIC_READ,
                                              FILE_SHARE_READ,
                                              OPEN_EXISTING,
                                              nullptr );
         if( handle != INVALID_HANDLE_VALUE ) {
            return handle;
         }
         const std::error_code ec( ::GetLastError(), std::system_category() );
         throw std::filesystem::filesystem_error( "CreateFile2() failed", m_path, ec );
#else
         const HANDLE handle = ::CreateFileW( m_path.c_str(),
                                              GENERIC_READ,
                                              FILE_SHARE_READ,
                                              nullptr,
                                              OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL,
                                              nullptr );
         if( handle != INVALID_HANDLE_VALUE ) {
            return handle;
         }
         const std::error_code ec( ::GetLastError(), std::system_category() );
         throw std::filesystem::filesystem_error( "CreateFileW()", m_path, ec );
#endif
      }
   };

   struct win32_file_mapper
   {
      explicit win32_file_mapper( const std::filesystem::path& path )
         : win32_file_mapper( win32_file_opener( path ) )
      {}

      explicit win32_file_mapper( const win32_file_opener& reader )
         : m_size( reader.size() ),
           m_handle( open( reader ) )
      {}

      win32_file_mapper( const win32_file_mapper& ) = delete;
      win32_file_mapper( win32_file_mapper&& ) = delete;

      ~win32_file_mapper()
      {
         ::CloseHandle( m_handle );
      }

      void operator=( const win32_file_mapper& ) = delete;
      void operator=( win32_file_mapper&& ) = delete;

      const size_t m_size;
      const HANDLE m_handle;

   private:
      [[nodiscard]] HANDLE open( const win32_file_opener& reader ) const
      {
         const uint64_t file_size = reader.size();
         SetLastError( 0 );
         // Use `CreateFileMappingW` because a) we're not specifying a
         // mapping name, so the character type is of no consequence, and
         // b) it's defined in `memoryapi.h`, unlike
         // `CreateFileMappingA`(?!)
         const HANDLE handle = ::CreateFileMappingW( reader.m_handle,
                                                     nullptr,
                                                     PAGE_READONLY,
                                                     DWORD( file_size >> 32 ),
                                                     DWORD( file_size & 0xffffffff ),
                                                     nullptr );
         if( handle != NULL || file_size == 0 ) {
            return handle;
         }
         const std::error_code ec( ::GetLastError(), std::system_category() );
         throw std::filesystem::filesystem_error( "CreateFileMappingW() failed", reader.m_path, ec );
      }
   };

   class file_mapper
   {
   public:
      explicit file_mapper( const std::filesystem::path& path )
         : file_mapper( win32_file_mapper( path ) )
      {}

      explicit file_mapper( const win32_file_mapper& mapper )
         : m_size( mapper.m_size ),
           m_data( static_cast< const char* >( ::MapViewOfFile( mapper.m_handle,
                                                                FILE_MAP_READ,
                                                                0,
                                                                0,
                                                                0 ) ) )
      {
         if( ( m_size != 0 ) && ( intptr_t( m_data ) == 0 ) ) {
            const std::error_code ec( ::GetLastError(), std::system_category() );
            throw std::filesystem::filesystem_error( "MapViewOfFile() failed", ec );
         }
      }

      file_mapper( const file_mapper& ) = delete;
      file_mapper( file_mapper&& ) = delete;

      ~file_mapper()
      {
         ::UnmapViewOfFile( LPCVOID( m_data ) );
      }

      void operator=( const file_mapper& ) = delete;
      void operator=( file_mapper&& ) = delete;

      [[nodiscard]] bool empty() const noexcept
      {
         return m_size == 0;
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return m_size;
      }

      using iterator = const char*;
      using const_iterator = const char*;

      [[nodiscard]] iterator data() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator begin() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] iterator end() const noexcept
      {
         return m_data + m_size;
      }

   private:
      const std::size_t m_size;
      const char* const m_data;
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 25 "tao/pegtl/mmap_input.hpp"
#else
#endif

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      struct mmap_holder
      {
         const file_mapper data;

         explicit mmap_holder( const std::filesystem::path& path )
            : data( path )
         {}

         mmap_holder( const mmap_holder& ) = delete;
         mmap_holder( mmap_holder&& ) = delete;

         ~mmap_holder() = default;

         void operator=( const mmap_holder& ) = delete;
         void operator=( mmap_holder&& ) = delete;
      };

   } // namespace internal

   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
   struct mmap_input
      : private internal::mmap_holder,
        public memory_input< P, Eol >
   {
      mmap_input( const std::filesystem::path& path, const std::string& source )
         : internal::mmap_holder( path ),
           memory_input< P, Eol >( data.begin(), data.end(), source )
      {}

      explicit mmap_input( const std::filesystem::path& path )
         : mmap_input( path, internal::path_to_string( path ) )
      {}

      mmap_input( const mmap_input& ) = delete;
      mmap_input( mmap_input&& ) = delete;

      ~mmap_input() = default;

      void operator=( const mmap_input& ) = delete;
      void operator=( mmap_input&& ) = delete;
   };

   template< typename... Ts >
   explicit mmap_input( Ts&&... ) -> mmap_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 17 "tao/pegtl/file_input.hpp"
#else

#endif

namespace TAO_PEGTL_NAMESPACE
{
#if defined( _POSIX_MAPPED_FILES ) || defined( _WIN32 )
   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
   struct file_input
      : mmap_input< P, Eol >
   {
      using mmap_input< P, Eol >::mmap_input;
   };
#else
   template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
   struct file_input
      : read_input< P, Eol >
   {
      using read_input< P, Eol >::read_input;
   };
#endif

   template< typename... Ts >
   explicit file_input( Ts&&... ) -> file_input<>;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 26 "tao/pegtl.hpp"

#line 1 "tao/pegtl/change_action.hpp"
       
#line 1 "tao/pegtl/change_action.hpp"



#ifndef TAO_PEGTL_CHANGE_ACTION_HPP
#define TAO_PEGTL_CHANGE_ACTION_HPP

#include <type_traits>






namespace TAO_PEGTL_NAMESPACE
{
   template< template< typename... > class NewAction >
   struct change_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         static_assert( !std::is_same_v< Action< void >, NewAction< void > >, "old and new action class templates are identical" );
         return Control< Rule >::template match< A, M, NewAction, Control >( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 28 "tao/pegtl.hpp"
#line 1 "tao/pegtl/change_action_and_state.hpp"
       
#line 1 "tao/pegtl/change_action_and_state.hpp"



#ifndef TAO_PEGTL_CHANGE_ACTION_AND_STATE_HPP
#define TAO_PEGTL_CHANGE_ACTION_AND_STATE_HPP

#include <type_traits>







namespace TAO_PEGTL_NAMESPACE
{
   template< template< typename... > class NewAction, typename NewState >
   struct change_action_and_state
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         static_assert( !std::is_same_v< Action< void >, NewAction< void > >, "old and new action class templates are identical" );
         NewState s( static_cast< const ParseInput& >( in ), st... );
         if( Control< Rule >::template match< A, M, NewAction, Control >( in, s ) ) {
            if constexpr( A == apply_mode::action ) {
               Action< Rule >::success( static_cast< const ParseInput& >( in ), s, st... );
            }
            return true;
         }
         return false;
      }

      template< typename ParseInput,
                typename... States >
      static void success( const ParseInput& in, NewState& s, States&&... st ) noexcept( noexcept( s.success( in, st... ) ) )
      {
         s.success( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 29 "tao/pegtl.hpp"
#line 1 "tao/pegtl/change_action_and_states.hpp"
       
#line 1 "tao/pegtl/change_action_and_states.hpp"



#ifndef TAO_PEGTL_CHANGE_ACTION_AND_STATES_HPP
#define TAO_PEGTL_CHANGE_ACTION_AND_STATES_HPP

#include <tuple>
#include <utility>







namespace TAO_PEGTL_NAMESPACE
{
   template< template< typename... > class NewAction, typename... NewStates >
   struct change_action_and_states
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                std::size_t... Ns,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( std::index_sequence< Ns... > /*unused*/, ParseInput& in, States&&... st )
      {
         auto t = std::tie( st... );
         if( Control< Rule >::template match< A, M, NewAction, Control >( in, std::get< Ns >( t )... ) ) {
            if constexpr( A == apply_mode::action ) {
               Action< Rule >::success( static_cast< const ParseInput& >( in ), st... );
            }
            return true;
         }
         return false;
      }

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         static_assert( !std::is_same_v< Action< void >, NewAction< void > >, "old and new action class templates are identical" );
         return match< Rule, A, M, Action, Control >( std::index_sequence_for< NewStates... >(), in, NewStates()..., st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 30 "tao/pegtl.hpp"
#line 1 "tao/pegtl/change_control.hpp"
       
#line 1 "tao/pegtl/change_control.hpp"



#ifndef TAO_PEGTL_CHANGE_CONTROL_HPP
#define TAO_PEGTL_CHANGE_CONTROL_HPP







namespace TAO_PEGTL_NAMESPACE
{
   template< template< typename... > class NewControl >
   struct change_control
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, NewControl >( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 31 "tao/pegtl.hpp"
#line 1 "tao/pegtl/change_state.hpp"
       
#line 1 "tao/pegtl/change_state.hpp"



#ifndef TAO_PEGTL_CHANGE_STATE_HPP
#define TAO_PEGTL_CHANGE_STATE_HPP







namespace TAO_PEGTL_NAMESPACE
{
   template< typename NewState >
   struct change_state
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         NewState s( static_cast< const ParseInput& >( in ), st... );
         if( TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, s ) ) {
            if constexpr( A == apply_mode::action ) {
               Action< Rule >::success( static_cast< const ParseInput& >( in ), s, st... );
            }
            return true;
         }
         return false;
      }

      template< typename ParseInput,
                typename... States >
      static void success( const ParseInput& in, NewState& s, States&&... st ) noexcept( noexcept( s.success( in, st... ) ) )
      {
         s.success( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 32 "tao/pegtl.hpp"
#line 1 "tao/pegtl/change_states.hpp"
       
#line 1 "tao/pegtl/change_states.hpp"



#ifndef TAO_PEGTL_CHANGE_STATES_HPP
#define TAO_PEGTL_CHANGE_STATES_HPP

#include <tuple>
#include <utility>







namespace TAO_PEGTL_NAMESPACE
{
   template< typename... NewStates >
   struct change_states
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                std::size_t... Ns,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( std::index_sequence< Ns... > /*unused*/, ParseInput& in, States&&... st )
      {
         auto t = std::tie( st... );
         if( TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, std::get< Ns >( t )... ) ) {
            if constexpr( A == apply_mode::action ) {
               Action< Rule >::success( static_cast< const ParseInput& >( in ), st... );
            }
            return true;
         }
         return false;
      }

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return match< Rule, A, M, Action, Control >( std::index_sequence_for< NewStates... >(), in, NewStates()..., st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 33 "tao/pegtl.hpp"

#line 1 "tao/pegtl/disable_action.hpp"
       
#line 1 "tao/pegtl/disable_action.hpp"



#ifndef TAO_PEGTL_DISABLE_ACTION_HPP
#define TAO_PEGTL_DISABLE_ACTION_HPP







namespace TAO_PEGTL_NAMESPACE
{
   struct disable_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return TAO_PEGTL_NAMESPACE::match< Rule, apply_mode::nothing, M, Action, Control >( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 35 "tao/pegtl.hpp"
#line 1 "tao/pegtl/enable_action.hpp"
       
#line 1 "tao/pegtl/enable_action.hpp"



#ifndef TAO_PEGTL_ENABLE_ACTION_HPP
#define TAO_PEGTL_ENABLE_ACTION_HPP







namespace TAO_PEGTL_NAMESPACE
{
   struct enable_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         return TAO_PEGTL_NAMESPACE::match< Rule, apply_mode::action, M, Action, Control >( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 36 "tao/pegtl.hpp"

#line 1 "tao/pegtl/discard_input.hpp"
       
#line 1 "tao/pegtl/discard_input.hpp"



#ifndef TAO_PEGTL_DISCARD_INPUT_HPP
#define TAO_PEGTL_DISCARD_INPUT_HPP







namespace TAO_PEGTL_NAMESPACE
{
   struct discard_input
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         const bool result = TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         in.discard();
         return result;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 38 "tao/pegtl.hpp"
#line 1 "tao/pegtl/discard_input_on_failure.hpp"
       
#line 1 "tao/pegtl/discard_input_on_failure.hpp"



#ifndef TAO_PEGTL_DISCARD_INPUT_ON_FAILURE_HPP
#define TAO_PEGTL_DISCARD_INPUT_ON_FAILURE_HPP







namespace TAO_PEGTL_NAMESPACE
{
   struct discard_input_on_failure
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         const bool result = TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         if( !result ) {
            in.discard();
         }
         return result;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 39 "tao/pegtl.hpp"
#line 1 "tao/pegtl/discard_input_on_success.hpp"
       
#line 1 "tao/pegtl/discard_input_on_success.hpp"



#ifndef TAO_PEGTL_DISCARD_INPUT_ON_SUCCESS_HPP
#define TAO_PEGTL_DISCARD_INPUT_ON_SUCCESS_HPP







namespace TAO_PEGTL_NAMESPACE
{
   struct discard_input_on_success
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         const bool result = TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         if( result ) {
            in.discard();
         }
         return result;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 40 "tao/pegtl.hpp"

#line 1 "tao/pegtl/must_if.hpp"
       
#line 1 "tao/pegtl/must_if.hpp"



#ifndef TAO_PEGTL_MUST_IF_HPP
#define TAO_PEGTL_MUST_IF_HPP

#include <type_traits>




namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename Errors, typename Rule, typename = void >
      inline constexpr bool raise_on_failure = ( Errors::template message< Rule > != nullptr );

      template< typename Errors, typename Rule >
      inline constexpr bool raise_on_failure< Errors, Rule, std::void_t< decltype( Errors::template raise_on_failure< Rule > ) > > = Errors::template raise_on_failure< Rule >;

   } // namespace internal

   template< typename Errors, template< typename... > class Base = normal, bool RequireMessage = true >
   struct must_if
   {
      template< typename Rule >
      struct control
         : Base< Rule >
      {
         template< typename ParseInput, typename... States >
         static void failure( const ParseInput& in, States&&... st ) noexcept( noexcept( Base< Rule >::failure( in, st... ) ) && !internal::raise_on_failure< Errors, Rule > )
         {
            if constexpr( internal::raise_on_failure< Errors, Rule > ) {
               raise( in, st... );
            }
            else {
               Base< Rule >::failure( in, st... );
            }
         }

         template< typename ParseInput, typename... States >
         [[noreturn]] static void raise( const ParseInput& in, [[maybe_unused]] States&&... st )
         {
            if constexpr( RequireMessage ) {
               static_assert( Errors::template message< Rule > != nullptr );
            }
            if constexpr( Errors::template message< Rule > != nullptr ) {
               constexpr const char* p = Errors::template message< Rule >;
               throw parse_error( p, in );
#if defined( _MSC_VER )
               ( (void)st, ... );
#endif
            }
            else {
               Base< Rule >::raise( in, st... );
            }
         }
      };
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 42 "tao/pegtl.hpp"
#line 1 "tao/pegtl/visit.hpp"
       
#line 1 "tao/pegtl/visit.hpp"



#ifndef TAO_PEGTL_VISIT_HPP
#define TAO_PEGTL_VISIT_HPP

#include <type_traits>




namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename Type, typename... Types >
      inline constexpr bool contains = ( std::is_same_v< Type, Types > || ... );

      template< typename Rules, typename Todo, typename Done >
      struct filter
      {
         using type = Todo;
      };

      template< typename Rule, typename... Rules, typename... Todo, typename... Done >
      struct filter< type_list< Rule, Rules... >, type_list< Todo... >, type_list< Done... > >
         : filter< type_list< Rules... >, std::conditional_t< contains< Rule, Todo..., Done... >, type_list< Todo... >, type_list< Rule, Todo... > >, type_list< Done... > >
      {};

      template< typename Rules, typename Todo, typename Done >
      using filter_t = typename filter< Rules, Todo, Done >::type;

      template< template< typename... > class Func, typename Done, typename... Rules >
      struct visitor
      {
         template< typename... Args >
         static void visit( Args&&... args )
         {
            ( Func< Rules >::visit( args... ), ... );
            using NextDone = type_list_concat_t< type_list< Rules... >, Done >;
            using NextSubs = type_list_concat_t< typename Rules::subs_t... >;
            using NextTodo = filter_t< NextSubs, empty_list, NextDone >;
            if constexpr( !std::is_same_v< NextTodo, empty_list > ) {
               visit_next< NextDone >( NextTodo(), args... );
            }
         }

      private:
         template< typename NextDone, typename... NextTodo, typename... Args >
         static void visit_next( type_list< NextTodo... > /*unused*/, Args&&... args )
         {
            visitor< Func, NextDone, NextTodo... >::visit( args... );
         }
      };

   } // namespace internal

   template< typename Rule, template< typename... > class Func, typename... Args >
   void visit( Args&&... args )
   {
      internal::visitor< Func, empty_list, Rule >::visit( args... );
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 43 "tao/pegtl.hpp"

#endif
#line 2 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/abnf.hpp"
       
#line 1 "tao/pegtl/contrib/abnf.hpp"



#ifndef TAO_PEGTL_CONTRIB_ABNF_HPP
#define TAO_PEGTL_CONTRIB_ABNF_HPP




namespace TAO_PEGTL_NAMESPACE::abnf
{
   // Core ABNF rules according to RFC 5234, Appendix B

   // clang-format off
   struct ALPHA : internal::ranges< internal::peek_char, 'a', 'z', 'A', 'Z' > {};
   struct BIT : internal::one< internal::result_on_found::success, internal::peek_char, '0', '1' > {};
   struct CHAR : internal::range< internal::result_on_found::success, internal::peek_char, char( 1 ), char( 127 ) > {};
   struct CR : internal::one< internal::result_on_found::success, internal::peek_char, '\r' > {};
   struct CRLF : internal::string< '\r', '\n' > {};
   struct CTL : internal::ranges< internal::peek_char, char( 0 ), char( 31 ), char( 127 ) > {};
   struct DIGIT : internal::range< internal::result_on_found::success, internal::peek_char, '0', '9' > {};
   struct DQUOTE : internal::one< internal::result_on_found::success, internal::peek_char, '"' > {};
   struct HEXDIG : internal::ranges< internal::peek_char, '0', '9', 'a', 'f', 'A', 'F' > {};
   struct HTAB : internal::one< internal::result_on_found::success, internal::peek_char, '\t' > {};
   struct LF : internal::one< internal::result_on_found::success, internal::peek_char, '\n' > {};
   struct LWSP : internal::star< internal::sor< internal::string< '\r', '\n' >, internal::one< internal::result_on_found::success, internal::peek_char, ' ', '\t' > >, internal::one< internal::result_on_found::success, internal::peek_char, ' ', '\t' > > {};
   struct OCTET : internal::any< internal::peek_char > {};
   struct SP : internal::one< internal::result_on_found::success, internal::peek_char, ' ' > {};
   struct VCHAR : internal::range< internal::result_on_found::success, internal::peek_char, char( 33 ), char( 126 ) > {};
   struct WSP : internal::one< internal::result_on_found::success, internal::peek_char, ' ', '\t' > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::abnf

#endif
#line 3 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/alphabet.hpp"
       
#line 1 "tao/pegtl/contrib/alphabet.hpp"



#ifndef TAO_PEGTL_CONTRIB_ALPHABET_HPP
#define TAO_PEGTL_CONTRIB_ALPHABET_HPP



namespace TAO_PEGTL_NAMESPACE::alphabet
{
   static const char a = 'a';
   static const char b = 'b';
   static const char c = 'c';
   static const char d = 'd';
   static const char e = 'e';
   static const char f = 'f';
   static const char g = 'g';
   static const char h = 'h';
   static const char i = 'i';
   static const char j = 'j';
   static const char k = 'k';
   static const char l = 'l';
   static const char m = 'm';
   static const char n = 'n';
   static const char o = 'o';
   static const char p = 'p';
   static const char q = 'q';
   static const char r = 'r';
   static const char s = 's';
   static const char t = 't';
   static const char u = 'u';
   static const char v = 'v';
   static const char w = 'w';
   static const char x = 'x';
   static const char y = 'y';
   static const char z = 'z';

   static const char A = 'A'; // NOLINT(readability-identifier-naming)
   static const char B = 'B'; // NOLINT(readability-identifier-naming)
   static const char C = 'C'; // NOLINT(readability-identifier-naming)
   static const char D = 'D'; // NOLINT(readability-identifier-naming)
   static const char E = 'E'; // NOLINT(readability-identifier-naming)
   static const char F = 'F'; // NOLINT(readability-identifier-naming)
   static const char G = 'G'; // NOLINT(readability-identifier-naming)
   static const char H = 'H'; // NOLINT(readability-identifier-naming)
   static const char I = 'I'; // NOLINT(readability-identifier-naming)
   static const char J = 'J'; // NOLINT(readability-identifier-naming)
   static const char K = 'K'; // NOLINT(readability-identifier-naming)
   static const char L = 'L'; // NOLINT(readability-identifier-naming)
   static const char M = 'M'; // NOLINT(readability-identifier-naming)
   static const char N = 'N'; // NOLINT(readability-identifier-naming)
   static const char O = 'O'; // NOLINT(readability-identifier-naming)
   static const char P = 'P'; // NOLINT(readability-identifier-naming)
   static const char Q = 'Q'; // NOLINT(readability-identifier-naming)
   static const char R = 'R'; // NOLINT(readability-identifier-naming)
   static const char S = 'S'; // NOLINT(readability-identifier-naming)
   static const char T = 'T'; // NOLINT(readability-identifier-naming)
   static const char U = 'U'; // NOLINT(readability-identifier-naming)
   static const char V = 'V'; // NOLINT(readability-identifier-naming)
   static const char W = 'W'; // NOLINT(readability-identifier-naming)
   static const char X = 'X'; // NOLINT(readability-identifier-naming)
   static const char Y = 'Y'; // NOLINT(readability-identifier-naming)
   static const char Z = 'Z'; // NOLINT(readability-identifier-naming)

} // namespace TAO_PEGTL_NAMESPACE::alphabet

#endif
#line 4 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/analyze.hpp"
       
#line 1 "tao/pegtl/contrib/analyze.hpp"



#ifndef TAO_PEGTL_CONTRIB_ANALYZE_HPP
#define TAO_PEGTL_CONTRIB_ANALYZE_HPP

#include <cassert>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>




#line 1 "tao/pegtl/contrib/analyze_traits.hpp"
       
#line 1 "tao/pegtl/contrib/analyze_traits.hpp"



#ifndef TAO_PEGTL_CONTRIB_ANALYZE_TRAITS_HPP
#define TAO_PEGTL_CONTRIB_ANALYZE_TRAITS_HPP

#include <type_traits>






#line 1 "tao/pegtl/contrib/forward.hpp"
       
#line 1 "tao/pegtl/contrib/forward.hpp"



#ifndef TAO_PEGTL_CONTRIB_FORWARD_HPP
#define TAO_PEGTL_CONTRIB_FORWARD_HPP



namespace TAO_PEGTL_NAMESPACE
{
   template< typename Name, typename Rule, typename = void >
   struct analyze_traits;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 15 "tao/pegtl/contrib/analyze_traits.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      enum class analyze_type
      {
         any, // Consumption-on-success is always true; assumes bounded repetition of conjunction of sub-rules.
         opt, // Consumption-on-success not necessarily true; assumes bounded repetition of conjunction of sub-rules.
         seq, // Consumption-on-success depends on consumption of (non-zero bounded repetition of) conjunction of sub-rules.
         sor // Consumption-on-success depends on consumption of (non-zero bounded repetition of) disjunction of sub-rules.
      };

   } // namespace internal

   template< typename... Rules >
   struct analyze_any_traits
   {
      static constexpr internal::analyze_type type_v = internal::analyze_type::any;
      using subs_t = type_list< Rules... >;
   };

   template< typename... Rules >
   struct analyze_opt_traits
   {
      static constexpr internal::analyze_type type_v = internal::analyze_type::opt;
      using subs_t = type_list< Rules... >;
   };

   template< typename... Rules >
   struct analyze_seq_traits
   {
      static constexpr internal::analyze_type type_v = internal::analyze_type::seq;
      using subs_t = type_list< Rules... >;
   };

   template< typename... Rules >
   struct analyze_sor_traits
   {
      static constexpr internal::analyze_type type_v = internal::analyze_type::sor;
      using subs_t = type_list< Rules... >;
   };

   template< typename Name, template< typename... > class Action, typename... Rules >
   struct analyze_traits< Name, internal::action< Action, Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name, typename Peek >
   struct analyze_traits< Name, internal::any< Peek > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename... Actions >
   struct analyze_traits< Name, internal::apply< Actions... > >
      : analyze_opt_traits<>
   {};

   template< typename Name, typename... Actions >
   struct analyze_traits< Name, internal::apply0< Actions... > >
      : analyze_opt_traits<>
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::at< Rules... > >
      : analyze_traits< Name, typename opt< Rules... >::rule_t >
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::bof >
      : analyze_opt_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::bol >
      : analyze_opt_traits<>
   {};

   template< typename Name, unsigned Cnt >
   struct analyze_traits< Name, internal::bytes< Cnt > >
      : std::conditional_t< ( Cnt != 0 ), analyze_any_traits<>, analyze_opt_traits<> >
   {};

   template< typename Name, template< typename... > class Control, typename... Rules >
   struct analyze_traits< Name, internal::control< Control, Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::disable< Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::discard >
      : analyze_opt_traits<>
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::enable< Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::eof >
      : analyze_opt_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::eol >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::eolf >
      : analyze_opt_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::failure >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Rule, typename... Actions >
   struct analyze_traits< Name, internal::if_apply< Rule, Actions... > >
      : analyze_traits< Name, typename Rule::rule_t >
   {};

   template< typename Name, typename Cond, typename... Rules >
   struct analyze_traits< Name, internal::if_must< true, Cond, Rules... > >
      : analyze_traits< Name, typename opt< Cond, Rules... >::rule_t >
   {};

   template< typename Name, typename Cond, typename... Rules >
   struct analyze_traits< Name, internal::if_must< false, Cond, Rules... > >
      : analyze_traits< Name, typename seq< Cond, Rules... >::rule_t >
   {};

   template< typename Name, typename Cond, typename Then, typename Else >
   struct analyze_traits< Name, internal::if_then_else< Cond, Then, Else > >
      : analyze_traits< Name, typename sor< seq< Cond, Then >, Else >::rule_t >
   {};

   template< typename Name, char... Cs >
   struct analyze_traits< Name, internal::istring< Cs... > >
      : std::conditional_t< ( sizeof...( Cs ) != 0 ), analyze_any_traits<>, analyze_opt_traits<> >
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::must< Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::not_at< Rules... > >
      : analyze_traits< Name, typename opt< Rules... >::rule_t >
   {};

   template< typename Name, internal::result_on_found R, typename Peek, typename Peek::data_t... Cs >
   struct analyze_traits< Name, internal::one< R, Peek, Cs... > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Rule, typename... Rules >
   struct analyze_traits< Name, internal::opt< Rule, Rules... > >
      : analyze_opt_traits< Rule, Rules... >
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::plus< Rules... > >
      : analyze_traits< Name, typename seq< Rules..., opt< Name > >::rule_t >
   {};

   template< typename Name, typename T >
   struct analyze_traits< Name, internal::raise< T > >
      : analyze_any_traits<>
   {};

   template< typename Name, internal::result_on_found R, typename Peek, typename Peek::data_t Lo, typename Peek::data_t Hi >
   struct analyze_traits< Name, internal::range< R, Peek, Lo, Hi > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Peek, typename Peek::data_t... Cs >
   struct analyze_traits< Name, internal::ranges< Peek, Cs... > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Head, typename... Rules >
   struct analyze_traits< Name, internal::rematch< Head, Rules... > >
      : analyze_traits< Name, typename sor< Head, sor< seq< Rules, any >... > >::rule_t > // TODO: Correct (enough)?
   {};

   template< typename Name, unsigned Cnt, typename... Rules >
   struct analyze_traits< Name, internal::rep< Cnt, Rules... > >
      : analyze_traits< Name, std::conditional_t< ( Cnt != 0 ), typename seq< Rules... >::rule_t, typename opt< Rules... >::rule_t > >
   {};

   template< typename Name, unsigned Min, unsigned Max, typename... Rules >
   struct analyze_traits< Name, internal::rep_min_max< Min, Max, Rules... > >
      : analyze_traits< Name, std::conditional_t< ( Min != 0 ), typename seq< Rules... >::rule_t, typename opt< Rules... >::rule_t > >
   {};

   template< typename Name, unsigned Max, typename... Rules >
   struct analyze_traits< Name, internal::rep_opt< Max, Rules... > >
      : analyze_traits< Name, typename opt< Rules... >::rule_t >
   {};

   template< typename Name, unsigned Amount >
   struct analyze_traits< Name, internal::require< Amount > >
      : analyze_opt_traits<>
   {};

   template< typename Name, typename Rule, typename... Rules >
   struct analyze_traits< Name, internal::seq< Rule, Rules... > >
      : analyze_seq_traits< Rule, Rules... >
   {};

   template< typename Name, typename Rule, typename... Rules >
   struct analyze_traits< Name, internal::sor< Rule, Rules... > >
      : analyze_sor_traits< Rule, Rules... >
   {};

   template< typename Name, typename... Rules >
   struct analyze_traits< Name, internal::star< Rules... > >
      : analyze_traits< Name, typename opt< Rules..., Name >::rule_t >
   {};

   template< typename Name, typename State, typename... Rules >
   struct analyze_traits< Name, internal::state< State, Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name, char... Cs >
   struct analyze_traits< Name, internal::string< Cs... > >
      : std::conditional_t< ( sizeof...( Cs ) != 0 ), analyze_any_traits<>, analyze_opt_traits<> >
   {};

   template< typename Name >
   struct analyze_traits< Name, internal::success >
      : analyze_opt_traits<>
   {};

   template< typename Name, typename Exception, typename... Rules >
   struct analyze_traits< Name, internal::try_catch_type< Exception, Rules... > >
      : analyze_traits< Name, typename seq< Rules... >::rule_t >
   {};

   template< typename Name, typename Cond >
   struct analyze_traits< Name, internal::until< Cond > >
      : analyze_traits< Name, typename Cond::rule_t >
   {};

   template< typename Name, typename Cond, typename... Rules >
   struct analyze_traits< Name, internal::until< Cond, Rules... > >
      : analyze_traits< Name, typename seq< star< Rules... >, Cond >::rule_t >
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 21 "tao/pegtl/contrib/analyze.hpp"

#line 1 "tao/pegtl/contrib/internal/set_stack_guard.hpp"
       
#line 1 "tao/pegtl/contrib/internal/set_stack_guard.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_SET_STACK_GUARD_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_SET_STACK_GUARD_HPP

#include <set>
#include <utility>



namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename... Cs >
   class set_stack_guard
   {
   public:
      template< typename... Ts >
      set_stack_guard( std::set< Cs... >& set, Ts&&... ts )
         : m_i( set.emplace( std::forward< Ts >( ts )... ) ),
           m_s( set )
      {}

      set_stack_guard( set_stack_guard&& ) = delete;
      set_stack_guard( const set_stack_guard& ) = delete;

      void operator=( set_stack_guard&& ) = delete;
      void operator=( const set_stack_guard& ) = delete;

      ~set_stack_guard()
      {
         if( m_i.second ) {
            m_s.erase( m_i.first );
         }
      }

      explicit operator bool() const noexcept
      {
         return m_i.second;
      }

   private:
      const std::pair< typename std::set< Cs... >::iterator, bool > m_i;
      std::set< Cs... >& m_s;
   };

   template< typename... Cs >
   set_stack_guard( std::set< Cs... >&, const typename std::set< Cs... >::value_type& ) -> set_stack_guard< Cs... >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 23 "tao/pegtl/contrib/analyze.hpp"



namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      struct analyze_entry
      {
         explicit analyze_entry( const analyze_type in_type ) noexcept
            : type( in_type )
         {}

         const analyze_type type;
         std::vector< std::string_view > subs;
      };

      class analyze_cycles_impl
      {
      public:
         analyze_cycles_impl( analyze_cycles_impl&& ) = delete;
         analyze_cycles_impl( const analyze_cycles_impl& ) = delete;

         ~analyze_cycles_impl() = default;

         void operator=( analyze_cycles_impl&& ) = delete;
         void operator=( const analyze_cycles_impl& ) = delete;

         [[nodiscard]] std::size_t problems()
         {
            for( auto i = m_info.begin(); i != m_info.end(); ++i ) {
               m_results[ i->first ] = work( i, false );
               m_cache.clear();
            }
            return m_problems;
         }

         template< typename Rule >
         [[nodiscard]] bool consumes() const
         {
            return m_results.at( demangle< Rule >() );
         }

      protected:
         explicit analyze_cycles_impl( const bool verbose ) noexcept
            : m_verbose( verbose ),
              m_problems( 0 )
         {}

         [[nodiscard]] std::map< std::string_view, analyze_entry >::const_iterator find( const std::string_view name ) const noexcept
         {
            const auto iter = m_info.find( name );
            assert( iter != m_info.end() );
            return iter;
         }

         [[nodiscard]] bool work( const std::map< std::string_view, analyze_entry >::const_iterator& start, const bool accum )
         {
            if( const auto j = m_cache.find( start->first ); j != m_cache.end() ) {
               return j->second;
            }
            if( const auto g = set_stack_guard( m_stack, start->first ) ) {
               switch( start->second.type ) {
                  case analyze_type::any: {
                     bool a = false;
                     for( const auto& r : start->second.subs ) {
                        a = a || work( find( r ), accum || a );
                     }
                     return m_cache[ start->first ] = true;
                  }
                  case analyze_type::opt: {
                     bool a = false;
                     for( const auto& r : start->second.subs ) {
                        a = a || work( find( r ), accum || a );
                     }
                     return m_cache[ start->first ] = false;
                  }
                  case analyze_type::seq: {
                     bool a = false;
                     for( const auto& r : start->second.subs ) {
                        a = a || work( find( r ), accum || a );
                     }
                     return m_cache[ start->first ] = a;
                  }
                  case analyze_type::sor: {
                     bool a = true;
                     for( const auto& r : start->second.subs ) {
                        a = a && work( find( r ), accum );
                     }
                     return m_cache[ start->first ] = a;
                  }
               }
               assert( false ); // LCOV_EXCL_LINE
            }
            if( !accum ) {
               ++m_problems;
               if( m_verbose ) {
                  std::cerr << "problem: cycle without progress detected at rule class " << start->first << std::endl; // LCOV_EXCL_LINE
               }
            }
            return m_cache[ start->first ] = accum;
         }

         const bool m_verbose;

         std::size_t m_problems;

         std::map< std::string_view, analyze_entry > m_info;
         std::set< std::string_view > m_stack;
         std::map< std::string_view, bool > m_cache;
         std::map< std::string_view, bool > m_results;
      };

      template< typename Name >
      std::string_view analyze_insert( std::map< std::string_view, analyze_entry >& info )
      {
         using Traits = analyze_traits< Name, typename Name::rule_t >;

         const auto [ i, b ] = info.try_emplace( demangle< Name >(), Traits::type_v );
         if( b ) {
            analyze_insert_impl( typename Traits::subs_t(), i->second.subs, info );
         }
         return i->first;
      }

      template< typename... Subs >
      void analyze_insert_impl( type_list< Subs... > /*unused*/, std::vector< std::string_view >& subs, std::map< std::string_view, analyze_entry >& info )
      {
         ( subs.emplace_back( analyze_insert< Subs >( info ) ), ... );
      }

      template< typename Grammar >
      class analyze_cycles
         : public analyze_cycles_impl
      {
      public:
         explicit analyze_cycles( const bool verbose )
            : analyze_cycles_impl( verbose )
         {
            analyze_insert< Grammar >( m_info );
         }
      };

   } // namespace internal

   template< typename Grammar >
   [[nodiscard]] std::size_t analyze( const bool verbose = true )
   {
      return internal::analyze_cycles< Grammar >( verbose ).problems();
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 5 "amalgamated.hpp"

#line 1 "tao/pegtl/contrib/control_action.hpp"
       
#line 1 "tao/pegtl/contrib/control_action.hpp"



#ifndef TAO_PEGTL_CONTRIB_CONTROL_ACTION_HPP
#define TAO_PEGTL_CONTRIB_CONTROL_ACTION_HPP

#include <utility>





namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename, typename Rule, template< typename... > class Action, typename ParseInput, typename... States >
      inline constexpr bool action_has_unwind = false;

      template< typename Rule, template< typename... > class Action, typename ParseInput, typename... States >
      inline constexpr bool action_has_unwind< decltype( (void)Action< Rule >::unwind( std::declval< const ParseInput& >(), std::declval< States&& >()... ) ), Rule, Action, ParseInput, States... > = true;

   } // namespace internal

   struct control_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( internal::action_has_unwind< void, Rule, Action, ParseInput, States... > ) {
            try {
               return control_action::match_impl< Rule, A, M, Action, Control >( in, st... );
            }
            catch( ... ) {
               Action< Rule >::unwind( const_cast< const ParseInput& >( in ), st... );
               throw;
            }
         }
         else {
            return control_action::match_impl< Rule, A, M, Action, Control >( in, st... );
         }
      }

   private:
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match_impl( ParseInput& in, States&&... st )
      {
         Action< Rule >::start( const_cast< const ParseInput& >( in ), st... );
         if( TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... ) ) {
            Action< Rule >::success( const_cast< const ParseInput& >( in ), st... );
            return true;
         }
         Action< Rule >::failure( const_cast< const ParseInput& >( in ), st... );
         return false;
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 7 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/coverage.hpp"
       
#line 1 "tao/pegtl/contrib/coverage.hpp"



#ifndef TAO_PEGTL_CONTRIB_COVERAGE_HPP
#define TAO_PEGTL_CONTRIB_COVERAGE_HPP

#include <cstddef>
#include <map>
#include <string_view>
#include <vector>

#line 1 "tao/pegtl/contrib/state_control.hpp"
       
#line 1 "tao/pegtl/contrib/state_control.hpp"



#ifndef TAO_PEGTL_CONTRIB_STATE_CONTROL_HPP
#define TAO_PEGTL_CONTRIB_STATE_CONTROL_HPP

#include <type_traits>

#line 1 "tao/pegtl/contrib/shuffle_states.hpp"
       
#line 1 "tao/pegtl/contrib/shuffle_states.hpp"



#ifndef TAO_PEGTL_CONTRIB_SHUFFLE_STATES_HPP
#define TAO_PEGTL_CONTRIB_SHUFFLE_STATES_HPP

#include <tuple>
#include <type_traits>
#include <utility>





namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< std::size_t N >
      struct rotate_left
      {
         template< std::size_t I, std::size_t S >
         static constexpr std::size_t value = ( I + N ) % S;
      };

      template< std::size_t N >
      struct rotate_right
      {
         template< std::size_t I, std::size_t S >
         static constexpr std::size_t value = ( I + S - N ) % S;
      };

      struct reverse
      {
         template< std::size_t I, std::size_t S >
         static constexpr std::size_t value = ( S - 1 ) - I;
      };

   } // namespace internal

   // Applies 'Shuffle' to the states of start(), success(), failure(), raise(), apply(), and apply0()
   template< typename Base, typename Shuffle >
   struct shuffle_states
      : Base
   {
      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void start_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::start( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
      {
         Base::start( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& in, States&&... st ) noexcept( noexcept( start_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) ) )
      {
         start_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< typename ParseInput, typename State >
      static void start( const ParseInput& in, State&& st ) noexcept( noexcept( Base::start( in, st ) ) )
      {
         Base::start( in, st );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void success_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::success( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
      {
         Base::success( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& in, States&&... st ) noexcept( noexcept( success_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) ) )
      {
         success_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< typename ParseInput, typename State >
      static void success( const ParseInput& in, State&& st ) noexcept( noexcept( Base::success( in, st ) ) )
      {
         Base::success( in, st );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void failure_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::failure( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
      {
         Base::failure( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& in, States&&... st ) noexcept( noexcept( failure_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) ) )
      {
         failure_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< typename ParseInput, typename State >
      static void failure( const ParseInput& in, State&& st ) noexcept( noexcept( Base::failure( in, st ) ) )
      {
         Base::failure( in, st );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      [[noreturn]] static void raise_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
      {
         Base::raise( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... st )
      {
         raise_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< typename ParseInput, typename State >
      [[noreturn]] static void raise( const ParseInput& in, State&& st )
      {
         Base::raise( in, st );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static auto unwind_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
         -> std::enable_if_t< internal::has_unwind< Base, void, const ParseInput&, std::tuple_element_t< Shuffle::template value< Is, sizeof...( Is ) >, Tuple >... > >
      {
         Base::unwind( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static auto unwind( const ParseInput& in, States&&... st )
         -> decltype( unwind_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) )
      {
         unwind_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< typename ParseInput, typename State >
      static auto unwind( const ParseInput& in, State&& st )
         -> std::enable_if_t< internal::has_unwind< Base, void, const ParseInput&, State > >
      {
         Base::unwind( in, st );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename Tuple, std::size_t... Is >
      static auto apply_impl( const Iterator& begin, const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
         -> decltype( Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) )
      {
         return Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename... States >
      static auto apply( const Iterator& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) ) )
         -> decltype( apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) )
      {
         return apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename State >
      static auto apply( const Iterator& begin, const ParseInput& in, State&& st ) noexcept( noexcept( Base::template apply< Action >( begin, in, st ) ) )
         -> decltype( Base::template apply< Action >( begin, in, st ) )
      {
         return Base::template apply< Action >( begin, in, st );
      }

      template< template< typename... > class Action, typename ParseInput, typename Tuple, std::size_t... Is >
      static auto apply0_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
         -> decltype( Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) )
      {
         return Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... );
      }

      template< template< typename... > class Action, typename ParseInput, typename... States >
      static auto apply0( const ParseInput& in, States&&... st ) noexcept( noexcept( apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) ) )
         -> decltype( apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() ) )
      {
         return apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) >() );
      }

      template< template< typename... > class Action, typename ParseInput, typename State >
      static auto apply0( const ParseInput& in, State&& st ) noexcept( noexcept( Base::template apply0< Action >( in, st ) ) )
         -> decltype( Base::template apply0< Action >( in, st ) )
      {
         return Base::template apply0< Action >( in, st );
      }
   };

   template< typename Base, std::size_t N = 1 >
   using rotate_states_left = shuffle_states< Base, internal::rotate_left< N > >;

   template< typename Base, std::size_t N = 1 >
   using rotate_states_right = shuffle_states< Base, internal::rotate_right< N > >;

   template< typename Base >
   using reverse_states = shuffle_states< Base, internal::reverse >;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 10 "tao/pegtl/contrib/state_control.hpp"




namespace TAO_PEGTL_NAMESPACE
{
   template< template< typename... > class Control >
   struct state_control
   {
      template< typename Rule >
      struct control
         : Control< Rule >
      {
         static constexpr bool enable = true;

         template< typename ParseInput, typename State, typename... States >
         static void start( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] State& state, [[maybe_unused]] States&&... st )
         {
            if constexpr( Control< Rule >::enable ) {
               Control< Rule >::start( in, st... );
            }
            if constexpr( State::template enable< Rule > ) {
               state.template start< Rule >( in, st... );
            }
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
         }

         template< typename ParseInput, typename State, typename... States >
         static void success( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] State& state, [[maybe_unused]] States&&... st )
         {
            if constexpr( State::template enable< Rule > ) {
               state.template success< Rule >( in, st... );
            }
            if constexpr( Control< Rule >::enable ) {
               Control< Rule >::success( in, st... );
            }
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
         }

         template< typename ParseInput, typename State, typename... States >
         static void failure( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] State& state, [[maybe_unused]] States&&... st )
         {
            if constexpr( State::template enable< Rule > ) {
               state.template failure< Rule >( in, st... );
            }
            if constexpr( Control< Rule >::enable ) {
               Control< Rule >::failure( in, st... );
            }
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
         }

         template< typename ParseInput, typename State, typename... States >
         [[noreturn]] static void raise( const ParseInput& in, [[maybe_unused]] State& state, States&&... st )
         {
            if constexpr( State::template enable< Rule > ) {
               state.template raise< Rule >( in, st... );
            }
            Control< Rule >::raise( in, st... );
         }

         template< typename ParseInput, typename State, typename... States >
         static auto unwind( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] State& state, [[maybe_unused]] States&&... st )
            -> std::enable_if_t< State::template enable< Rule > || ( Control< Rule >::enable && internal::has_unwind< Control< Rule >, void, const ParseInput&, States... > ) >
         {
            if constexpr( State::template enable< Rule > ) {
               state.template unwind< Rule >( in, st... );
            }
            if constexpr( Control< Rule >::enable && internal::has_unwind< Control< Rule >, void, const ParseInput&, States... > ) {
               Control< Rule >::unwind( in, st... );
            }
#if defined( _MSC_VER )
            ( (void)st, ... );
#endif
         }

         template< template< typename... > class Action, typename Iterator, typename ParseInput, typename State, typename... States >
         static auto apply( const Iterator& begin, const ParseInput& in, [[maybe_unused]] State& state, States&&... st )
            -> decltype( Control< Rule >::template apply< Action >( begin, in, st... ) )
         {
            if constexpr( State::template enable< Rule > ) {
               state.template apply< Rule >( in, st... );
            }
            return Control< Rule >::template apply< Action >( begin, in, st... );
         }

         template< template< typename... > class Action, typename ParseInput, typename State, typename... States >
         static auto apply0( const ParseInput& in, [[maybe_unused]] State& state, States&&... st )
            -> decltype( Control< Rule >::template apply0< Action >( in, st... ) )
         {
            if constexpr( State::template enable< Rule > ) {
               state.template apply0< Rule >( in, st... );
            }
            return Control< Rule >::template apply0< Action >( in, st... );
         }
      };

      template< typename Rule >
      using type = rotate_states_right< control< Rule > >;
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 13 "tao/pegtl/contrib/coverage.hpp"
#line 24 "tao/pegtl/contrib/coverage.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   struct coverage_info
   {
      std::size_t start = 0;
      std::size_t success = 0;
      std::size_t failure = 0;
      std::size_t unwind = 0;
      std::size_t raise = 0;
   };

   struct coverage_entry
      : coverage_info
   {
      std::map< std::string_view, coverage_info > branches;
   };

   using coverage_result = std::map< std::string_view, coverage_entry >;

   namespace internal
   {
      template< typename Rule >
      struct coverage_insert
      {
         static void visit( std::map< std::string_view, coverage_entry >& map )
         {
            visit_branches( map.try_emplace( demangle< Rule >() ).first->second.branches, typename Rule::subs_t() );
         }

         template< typename... Ts >
         static void visit_branches( std::map< std::string_view, coverage_info >& branches, type_list< Ts... > /*unused*/ )
         {
            ( branches.try_emplace( demangle< Ts >() ), ... );
         }
      };

      struct coverage_state
      {
         template< typename Rule >
         static constexpr bool enable = true;

         explicit coverage_state( coverage_result& in_result )
            : result( in_result )
         {}

         coverage_result& result;
         std::vector< std::string_view > stack;

         template< typename Rule, typename ParseInput, typename... States >
         void start( const ParseInput& /*unused*/, States&&... /*unused*/ )
         {
            const auto name = demangle< Rule >();
            ++result.at( name ).start;
            if( !stack.empty() ) {
               ++result.at( stack.back() ).branches.at( name ).start;
            }
            stack.push_back( name );
         }

         template< typename Rule, typename ParseInput, typename... States >
         void success( const ParseInput& /*unused*/, States&&... /*unused*/ )
         {
            stack.pop_back();
            const auto name = demangle< Rule >();
            ++result.at( name ).success;
            if( !stack.empty() ) {
               ++result.at( stack.back() ).branches.at( name ).success;
            }
         }

         template< typename Rule, typename ParseInput, typename... States >
         void failure( const ParseInput& /*unused*/, States&&... /*unused*/ )
         {
            stack.pop_back();
            const auto name = demangle< Rule >();
            ++result.at( name ).failure;
            if( !stack.empty() ) {
               ++result.at( stack.back() ).branches.at( name ).failure;
            }
         }

         template< typename Rule, typename ParseInput, typename... States >
         void raise( const ParseInput& /*unused*/, States&&... /*unused*/ )
         {
            const auto name = demangle< Rule >();
            ++result.at( name ).raise;
            if( !stack.empty() ) {
               ++result.at( stack.back() ).branches.at( name ).raise;
            }
         }

         template< typename Rule, typename ParseInput, typename... States >
         void unwind( const ParseInput& /*unused*/, States&&... /*unused*/ )
         {
            stack.pop_back();
            const auto name = demangle< Rule >();
            ++result.at( name ).unwind;
            if( !stack.empty() ) {
               ++result.at( stack.back() ).branches.at( name ).unwind;
            }
         }

         template< typename Rule, typename ParseInput, typename... States >
         void apply( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
         {}

         template< typename Rule, typename ParseInput, typename... States >
         void apply0( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
         {}
      };

   } // namespace internal

   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   bool coverage( ParseInput&& in, coverage_result& result, States&&... st )
   {
      internal::coverage_state state( result );
      visit< Rule, internal::coverage_insert >( state.result ); // Fill map with all sub-rules of the grammar.
      return parse< Rule, Action, state_control< Control >::template type >( in, st..., state );
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 8 "amalgamated.hpp"

#line 1 "tao/pegtl/contrib/http.hpp"
       
#line 1 "tao/pegtl/contrib/http.hpp"



#ifndef TAO_PEGTL_CONTRIB_HTTP_HPP
#define TAO_PEGTL_CONTRIB_HTTP_HPP
#line 15 "tao/pegtl/contrib/http.hpp"
#line 1 "tao/pegtl/contrib/remove_first_state.hpp"
       
#line 1 "tao/pegtl/contrib/remove_first_state.hpp"



#ifndef TAO_PEGTL_CONTRIB_REMOVE_FIRST_STATE_HPP
#define TAO_PEGTL_CONTRIB_REMOVE_FIRST_STATE_HPP

#include <type_traits>





namespace TAO_PEGTL_NAMESPACE
{
   // Applies to start(), success(), failure(), raise(), apply(), and apply0():
   // The first state is removed when the call is forwarded to Base.
   template< typename Base >
   struct remove_first_state
      : Base
   {
      template< typename ParseInput, typename State, typename... States >
      static void start( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::start( in, st... ) ) )
      {
         Base::start( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static void success( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::success( in, st... ) ) )
      {
         Base::success( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static void failure( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::failure( in, st... ) ) )
      {
         Base::failure( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, State&& /*unused*/, States&&... st )
      {
         Base::raise( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static auto unwind( const ParseInput& in, State&& /*unused*/, States&&... st )
         -> std::enable_if_t< internal::has_unwind< Base, void, const ParseInput&, States... > >
      {
         Base::unwind( in, st... );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename State, typename... States >
      static auto apply( const Iterator& begin, const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::template apply< Action >( begin, in, st... ) ) )
         -> decltype( Base::template apply< Action >( begin, in, st... ) )
      {
         return Base::template apply< Action >( begin, in, st... );
      }

      template< template< typename... > class Action, typename ParseInput, typename State, typename... States >
      static auto apply0( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::template apply0< Action >( in, st... ) ) )
         -> decltype( Base::template apply0< Action >( in, st... ) )
      {
         return Base::template apply0< Action >( in, st... );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 16 "tao/pegtl/contrib/http.hpp"
#line 1 "tao/pegtl/contrib/uri.hpp"
       
#line 1 "tao/pegtl/contrib/uri.hpp"



#ifndef TAO_PEGTL_CONTRIB_URI_HPP
#define TAO_PEGTL_CONTRIB_URI_HPP

#include <cstdint>







#line 1 "tao/pegtl/contrib/integer.hpp"
       
#line 1 "tao/pegtl/contrib/integer.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTEGER_HPP
#define TAO_PEGTL_CONTRIB_INTEGER_HPP

#include <cstdint>
#include <cstdlib>

#include <limits>
#include <string_view>
#include <type_traits>
#line 21 "tao/pegtl/contrib/integer.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   struct unsigned_rule_old
      : plus< digit >
   {
      // Pre-3.0 version of this rule.
   };

   struct unsigned_rule_new
      : if_then_else< one< '0' >, not_at< digit >, plus< digit > >
   {
      // New version that does not allow leading zeros.
   };

   struct signed_rule_old
      : seq< opt< one< '-', '+' > >, plus< digit > >
   {
      // Pre-3.0 version of this rule.
   };

   struct signed_rule_new
      : seq< opt< one< '-', '+' > >, if_then_else< one< '0' >, not_at< digit >, plus< digit > > >
   {
      // New version that does not allow leading zeros.
   };

   struct signed_rule_bis
      : seq< opt< one< '-' > >, if_then_else< one< '0' >, not_at< digit >, plus< digit > > >
   {};

   struct signed_rule_ter
      : seq< one< '-', '+' >, if_then_else< one< '0' >, not_at< digit >, plus< digit > > >
   {};

   namespace internal
   {
      [[nodiscard]] constexpr bool is_digit( const char c ) noexcept
      {
         // We don't use std::isdigit() because it might
         // return true for other values on MS platforms.

         return ( '0' <= c ) && ( c <= '9' );
      }

      template< typename Integer, Integer Maximum = ( std::numeric_limits< Integer >::max )() >
      [[nodiscard]] constexpr bool accumulate_digit( Integer& result, const char digit ) noexcept
      {
         // Assumes that digit is a digit as per is_digit(); returns false on overflow.

         static_assert( std::is_integral_v< Integer > );

         constexpr Integer cutoff = Maximum / 10;
         constexpr Integer cutlim = Maximum % 10;

         const Integer c = digit - '0';

         if( ( result > cutoff ) || ( ( result == cutoff ) && ( c > cutlim ) ) ) {
            return false;
         }
         result *= 10;
         result += c;
         return true;
      }

      template< typename Integer, Integer Maximum = ( std::numeric_limits< Integer >::max )() >
      [[nodiscard]] constexpr bool accumulate_digits( Integer& result, const std::string_view input ) noexcept
      {
         // Assumes input is a non-empty sequence of digits; returns false on overflow.

         for( char c : input ) {
            if( !accumulate_digit< Integer, Maximum >( result, c ) ) {
               return false;
            }
         }
         return true;
      }

      template< typename Integer, Integer Maximum = ( std::numeric_limits< Integer >::max )() >
      [[nodiscard]] constexpr bool convert_positive( Integer& result, const std::string_view input ) noexcept
      {
         // Assumes result == 0 and that input is a non-empty sequence of digits; returns false on overflow.

         static_assert( std::is_integral_v< Integer > );
         return accumulate_digits< Integer, Maximum >( result, input );
      }

      template< typename Signed >
      [[nodiscard]] constexpr bool convert_negative( Signed& result, const std::string_view input ) noexcept
      {
         // Assumes result == 0 and that input is a non-empty sequence of digits; returns false on overflow.

         static_assert( std::is_signed_v< Signed > );
         using Unsigned = std::make_unsigned_t< Signed >;
         constexpr Unsigned maximum = static_cast< Unsigned >( ( std::numeric_limits< Signed >::max )() ) + 1;
         Unsigned temporary = 0;
         if( accumulate_digits< Unsigned, maximum >( temporary, input ) ) {
            result = static_cast< Signed >( ~temporary ) + 1;
            return true;
         }
         return false;
      }

      template< typename Unsigned, Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
      [[nodiscard]] constexpr bool convert_unsigned( Unsigned& result, const std::string_view input ) noexcept
      {
         // Assumes result == 0 and that input is a non-empty sequence of digits; returns false on overflow.

         static_assert( std::is_unsigned_v< Unsigned > );
         return accumulate_digits< Unsigned, Maximum >( result, input );
      }

      template< typename Signed >
      [[nodiscard]] constexpr bool convert_signed( Signed& result, const std::string_view input ) noexcept
      {
         // Assumes result == 0 and that input is an optional sign followed by a non-empty sequence of digits; returns false on overflow.

         static_assert( std::is_signed_v< Signed > );
         if( input[ 0 ] == '-' ) {
            return convert_negative< Signed >( result, std::string_view( input.data() + 1, input.size() - 1 ) );
         }
         const auto offset = unsigned( input[ 0 ] == '+' );
         return convert_positive< Signed >( result, std::string_view( input.data() + offset, input.size() - offset ) );
      }

      template< typename ParseInput >
      [[nodiscard]] bool match_unsigned( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( !in.empty() ) {
            const char c = in.peek_char();
            if( is_digit( c ) ) {
               in.bump_in_this_line();
               if( c == '0' ) {
                  return in.empty() || ( !is_digit( in.peek_char() ) );
               }
               while( ( !in.empty() ) && is_digit( in.peek_char() ) ) {
                  in.bump_in_this_line();
               }
               return true;
            }
         }
         return false;
      }

      template< typename ParseInput,
                typename Unsigned,
                Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
      [[nodiscard]] bool match_and_convert_unsigned_with_maximum( ParseInput& in, Unsigned& st )
      {
         // Assumes st == 0.

         if( !in.empty() ) {
            char c = in.peek_char();
            if( is_digit( c ) ) {
               if( c == '0' ) {
                  in.bump_in_this_line();
                  return in.empty() || ( !is_digit( in.peek_char() ) );
               }
               do {
                  if( !accumulate_digit< Unsigned, Maximum >( st, c ) ) {
                     throw TAO_PEGTL_NAMESPACE::parse_error( "integer overflow", in ); // Consistent with "as if" an action was doing the conversion.
                  }
                  in.bump_in_this_line();
               } while( ( !in.empty() ) && is_digit( c = in.peek_char() ) );
               return true;
            }
         }
         return false;
      }

   } // namespace internal

   struct unsigned_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits.

      template< typename ActionInput, typename Unsigned >
      static void apply( const ActionInput& in, Unsigned& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_unsigned( st, in.string_view() ) ) {
            throw parse_error( "unsigned integer overflow", in );
         }
      }
   };

   struct unsigned_rule
   {
      using rule_t = unsigned_rule;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         return internal::match_unsigned( in ); // Does not check for any overflow.
      }
   };

   struct unsigned_rule_with_action
   {
      using rule_t = unsigned_rule_with_action;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static auto match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) ) -> std::enable_if_t< A == apply_mode::nothing, bool >
      {
         return internal::match_unsigned( in ); // Does not check for any overflow.
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename Unsigned >
      [[nodiscard]] static auto match( ParseInput& in, Unsigned& st ) -> std::enable_if_t< ( A == apply_mode::action ) && std::is_unsigned_v< Unsigned >, bool >
      {
         // This function "only" offers basic exception safety.
         st = 0;
         return internal::match_and_convert_unsigned_with_maximum( in, st ); // Throws on overflow.
      }
   };

   template< typename Unsigned, Unsigned Maximum >
   struct maximum_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits.

      static_assert( std::is_unsigned_v< Unsigned > );

      template< typename ActionInput, typename Unsigned2 >
      static void apply( const ActionInput& in, Unsigned2& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_unsigned< Unsigned, Maximum >( st, in.string_view() ) ) {
            throw parse_error( "unsigned integer overflow", in );
         }
      }
   };

   template< typename Unsigned, Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
   struct maximum_rule
   {
      using rule_t = maximum_rule;
      using subs_t = empty_list;

      static_assert( std::is_unsigned_v< Unsigned > );

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in )
      {
         Unsigned st = 0;
         return internal::match_and_convert_unsigned_with_maximum< ParseInput, Unsigned, Maximum >( in, st ); // Throws on overflow.
      }
   };

   template< typename Unsigned, Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
   struct maximum_rule_with_action
   {
      using rule_t = maximum_rule_with_action;
      using subs_t = empty_list;

      static_assert( std::is_unsigned_v< Unsigned > );

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static auto match( ParseInput& in, States&&... /*unused*/ ) -> std::enable_if_t< A == apply_mode::nothing, bool >
      {
         Unsigned st = 0;
         return internal::match_and_convert_unsigned_with_maximum< ParseInput, Unsigned, Maximum >( in, st ); // Throws on overflow.
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename Unsigned2 >
      [[nodiscard]] static auto match( ParseInput& in, Unsigned2& st ) -> std::enable_if_t< ( A == apply_mode::action ) && std::is_same_v< Unsigned, Unsigned2 >, bool >
      {
         // This function "only" offers basic exception safety.
         st = 0;
         return internal::match_and_convert_unsigned_with_maximum< ParseInput, Unsigned, Maximum >( in, st ); // Throws on overflow.
      }
   };

   struct signed_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits,
      // with optional leading sign; with sign, in.size() must be >= 2.

      template< typename ActionInput, typename Signed >
      static void apply( const ActionInput& in, Signed& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_signed( st, in.string_view() ) ) {
            throw parse_error( "signed integer overflow", in );
         }
      }
   };

   struct signed_rule
   {
      using rule_t = signed_rule;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         return TAO_PEGTL_NAMESPACE::parse< signed_rule_new >( in ); // Does not check for any overflow.
      }
   };

   namespace internal
   {
      template< typename Rule >
      struct signed_action_action
         : nothing< Rule >
      {};

      template<>
      struct signed_action_action< signed_rule_new >
         : signed_action
      {};

   } // namespace internal

   struct signed_rule_with_action
   {
      using rule_t = signed_rule_with_action;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static auto match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) ) -> std::enable_if_t< A == apply_mode::nothing, bool >
      {
         return TAO_PEGTL_NAMESPACE::parse< signed_rule_new >( in ); // Does not check for any overflow.
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename Signed >
      [[nodiscard]] static auto match( ParseInput& in, Signed& st ) -> std::enable_if_t< ( A == apply_mode::action ) && std::is_signed_v< Signed >, bool >
      {
         return TAO_PEGTL_NAMESPACE::parse< signed_rule_new, internal::signed_action_action >( in, st ); // Throws on overflow.
      }
   };

   template< typename Name >
   struct analyze_traits< Name, unsigned_rule >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, unsigned_rule_with_action >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Integer, Integer Maximum >
   struct analyze_traits< Name, maximum_rule< Integer, Maximum > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Integer, Integer Maximum >
   struct analyze_traits< Name, maximum_rule_with_action< Integer, Maximum > >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, signed_rule >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, signed_rule_with_action >
      : analyze_any_traits<>
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 16 "tao/pegtl/contrib/uri.hpp"

namespace TAO_PEGTL_NAMESPACE::uri
{
   // URI grammar according to RFC 3986.

   // This grammar is a direct PEG translation of the original URI grammar.
   // It should be considered experimental -- in case of any issues, in particular
   // missing rules for attached actions, please contact the developers.

   // Note that this grammar has multiple top-level rules.

   using dot = one< '.' >;
   using colon = one< ':' >;

   // clang-format off
   struct dec_octet : maximum_rule< std::uint8_t > {};

   struct IPv4address : seq< dec_octet, dot, dec_octet, dot, dec_octet, dot, dec_octet > {};

   struct h16 : rep_min_max< 1, 4, abnf::HEXDIG > {};
   struct ls32 : sor< seq< h16, colon, h16 >, IPv4address > {};

   struct dcolon : two< ':' > {};

   struct IPv6address : sor< seq< rep< 6, h16, colon >, ls32 >,
                             seq< dcolon, rep< 5, h16, colon >, ls32 >,
                             seq< opt< h16 >, dcolon, rep< 4, h16, colon >, ls32 >,
                             seq< opt< h16, opt< colon, h16 > >, dcolon, rep< 3, h16, colon >, ls32 >,
                             seq< opt< h16, rep_opt< 2, colon, h16 > >, dcolon, rep< 2, h16, colon >, ls32 >,
                             seq< opt< h16, rep_opt< 3, colon, h16 > >, dcolon, h16, colon, ls32 >,
                             seq< opt< h16, rep_opt< 4, colon, h16 > >, dcolon, ls32 >,
                             seq< opt< h16, rep_opt< 5, colon, h16 > >, dcolon, h16 >,
                             seq< opt< h16, rep_opt< 6, colon, h16 > >, dcolon > > {};

   struct gen_delims : one< ':', '/', '?', '#', '[', ']', '@' > {};
   struct sub_delims : one< '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '=' > {};

   struct unreserved : sor< abnf::ALPHA, abnf::DIGIT, one< '-', '.', '_', '~' > > {};
   struct reserved : sor< gen_delims, sub_delims > {};

   struct IPvFuture : if_must< one< 'v', 'V' >, plus< abnf::HEXDIG >, dot, plus< sor< unreserved, sub_delims, colon > > > {};

   struct IP_literal : if_must< one< '[' >, sor< IPvFuture, IPv6address >, one< ']' > > {};

   struct pct_encoded : if_must< one< '%' >, abnf::HEXDIG, abnf::HEXDIG > {};
   struct pchar : sor< unreserved, pct_encoded, sub_delims, one< ':', '@' > > {};

   struct query : star< sor< pchar, one< '/', '?' > > > {};
   struct fragment : star< sor< pchar, one< '/', '?' > > > {};

   struct segment : star< pchar > {};
   struct segment_nz : plus< pchar > {};
   struct segment_nz_nc : plus< sor< unreserved, pct_encoded, sub_delims, one< '@' > > > {}; // non-zero-length segment without any colon ":"

   struct path_abempty : star< one< '/' >, segment > {};
   struct path_absolute : seq< one< '/' >, opt< segment_nz, star< one< '/' >, segment > > > {};
   struct path_noscheme : seq< segment_nz_nc, star< one< '/' >, segment > > {};
   struct path_rootless : seq< segment_nz, star< one< '/' >, segment > > {};
   struct path_empty : success {};

   struct path : sor< path_noscheme, // begins with a non-colon segment
                      path_rootless, // begins with a segment
                      path_absolute, // begins with "/" but not "//"
                      path_abempty > {}; // begins with "/" or is empty

   struct reg_name : star< sor< unreserved, pct_encoded, sub_delims > > {};

   struct port : star< abnf::DIGIT > {};
   struct host : sor< IP_literal, IPv4address, reg_name > {};
   struct userinfo : star< sor< unreserved, pct_encoded, sub_delims, colon > > {};
   struct opt_userinfo : opt< userinfo, one< '@' > > {};
   struct authority : seq< opt_userinfo, host, opt< colon, port > > {};

   struct scheme : seq< abnf::ALPHA, star< sor< abnf::ALPHA, abnf::DIGIT, one< '+', '-', '.' > > > > {};

   using dslash = two< '/' >;
   using opt_query = opt_must< one< '?' >, query >;
   using opt_fragment = opt_must< one< '#' >, fragment >;

   struct hier_part : sor< if_must< dslash, authority, path_abempty >, path_rootless, path_absolute, path_empty > {};
   struct relative_part : sor< if_must< dslash, authority, path_abempty >, path_noscheme, path_absolute, path_empty > {};
   struct relative_ref : seq< relative_part, opt_query, opt_fragment > {};

   struct URI : seq< scheme, one< ':' >, hier_part, opt_query, opt_fragment > {};
   struct URI_reference : sor< URI, relative_ref > {};
   struct absolute_URI : seq< scheme, one< ':' >, hier_part, opt_query > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::uri

#endif
#line 17 "tao/pegtl/contrib/http.hpp"

namespace TAO_PEGTL_NAMESPACE::http
{
   // HTTP 1.1 grammar according to RFC 7230.

   // This grammar is a direct PEG translation of the original HTTP grammar.
   // It should be considered experimental -- in case of any issues, in particular
   // missing rules for attached actions, please contact the developers.

   using OWS = star< abnf::WSP >; // optional whitespace
   using RWS = plus< abnf::WSP >; // required whitespace
   using BWS = OWS; // "bad" whitespace

   using obs_text = not_range< 0x00, 0x7F >;
   using obs_fold = seq< abnf::CRLF, plus< abnf::WSP > >;

   // clang-format off
   struct tchar : sor< abnf::ALPHA, abnf::DIGIT, one< '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~' > > {};
   struct token : plus< tchar > {};

   struct field_name : token {};

   struct field_vchar : sor< abnf::VCHAR, obs_text > {};
   struct field_content : list< field_vchar, plus< abnf::WSP > > {};
   struct field_value : star< sor< field_content, obs_fold > > {};

   struct header_field : seq< field_name, one< ':' >, OWS, field_value, OWS > {};

   struct method : token {};

   struct absolute_path : plus< one< '/' >, uri::segment > {};

   struct origin_form : seq< absolute_path, uri::opt_query > {};
   struct absolute_form : uri::absolute_URI {};
   struct authority_form : uri::authority {};
   struct asterisk_form : one< '*' > {};

   struct request_target : sor< origin_form, absolute_form, authority_form, asterisk_form > {};

   struct status_code : rep< 3, abnf::DIGIT > {};
   struct reason_phrase : star< sor< abnf::VCHAR, obs_text, abnf::WSP > > {};

   struct HTTP_version : if_must< string< 'H', 'T', 'T', 'P', '/' >, abnf::DIGIT, one< '.' >, abnf::DIGIT > {};

   struct request_line : if_must< method, abnf::SP, request_target, abnf::SP, HTTP_version, abnf::CRLF > {};
   struct status_line : if_must< HTTP_version, abnf::SP, status_code, abnf::SP, reason_phrase, abnf::CRLF > {};
   struct start_line : sor< status_line, request_line > {};

   struct message_body : star< abnf::OCTET > {};
   struct HTTP_message : seq< start_line, star< header_field, abnf::CRLF >, abnf::CRLF, opt< message_body > > {};

   struct Content_Length : plus< abnf::DIGIT > {};

   struct uri_host : uri::host {};
   struct port : uri::port {};

   struct Host : seq< uri_host, opt< one< ':' >, port > > {};

   // PEG are different from CFGs! (this replaces ctext and qdtext)
   using text = sor< abnf::HTAB, range< 0x20, 0x7E >, obs_text >;

   struct quoted_pair : if_must< one< '\\' >, sor< abnf::VCHAR, obs_text, abnf::WSP > > {};
   struct quoted_string : if_must< abnf::DQUOTE, until< abnf::DQUOTE, sor< quoted_pair, text > > > {};

   struct transfer_parameter : seq< token, BWS, one< '=' >, BWS, sor< token, quoted_string > > {};
   struct transfer_extension : seq< token, star< OWS, one< ';' >, OWS, transfer_parameter > > {};
   struct transfer_coding : sor< istring< 'c', 'h', 'u', 'n', 'k', 'e', 'd' >,
                                 istring< 'c', 'o', 'm', 'p', 'r', 'e', 's', 's' >,
                                 istring< 'd', 'e', 'f', 'l', 'a', 't', 'e' >,
                                 istring< 'g', 'z', 'i', 'p' >,
                                 transfer_extension > {};

   struct rank : sor< seq< one< '0' >, opt< one< '.' >, rep_opt< 3, abnf::DIGIT > > >,
                      seq< one< '1' >, opt< one< '.' >, rep_opt< 3, one< '0' > > > > > {};

   struct t_ranking : seq< OWS, one< ';' >, OWS, one< 'q', 'Q' >, one< '=' >, rank > {};
   struct t_codings : sor< istring< 't', 'r', 'a', 'i', 'l', 'e', 'r', 's' >, seq< transfer_coding, opt< t_ranking > > > {};

   struct TE : opt< sor< one< ',' >, t_codings >, star< OWS, one< ',' >, opt< OWS, t_codings > > > {};

   template< typename T >
   using make_comma_list = seq< star< one< ',' >, OWS >, T, star< OWS, one< ',' >, opt< OWS, T > > >;

   struct connection_option : token {};
   struct Connection : make_comma_list< connection_option > {};

   struct Trailer : make_comma_list< field_name > {};

   struct Transfer_Encoding : make_comma_list< transfer_coding > {};

   struct protocol_name : token {};
   struct protocol_version : token {};
   struct protocol : seq< protocol_name, opt< one< '/' >, protocol_version > > {};
   struct Upgrade : make_comma_list< protocol > {};

   struct pseudonym : token {};

   struct received_protocol : seq< opt< protocol_name, one< '/' > >, protocol_version > {};
   struct received_by : sor< seq< uri_host, opt< one< ':' >, port > >, pseudonym > {};

   struct comment : if_must< one< '(' >, until< one< ')' >, sor< comment, quoted_pair, text > > > {};

   struct Via : make_comma_list< seq< received_protocol, RWS, received_by, opt< RWS, comment > > > {};

   struct http_URI : if_must< istring< 'h', 't', 't', 'p', ':', '/', '/' >, uri::authority, uri::path_abempty, uri::opt_query, uri::opt_fragment > {};
   struct https_URI : if_must< istring< 'h', 't', 't', 'p', 's', ':', '/', '/' >, uri::authority, uri::path_abempty, uri::opt_query, uri::opt_fragment > {};

   struct partial_URI : seq< uri::relative_part, uri::opt_query > {};

   // clang-format on
   struct chunk_size
   {
      using rule_t = plus< abnf::HEXDIG >::rule_t;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, std::size_t& size, States&&... /*unused*/ )
      {
         size = 0;
         std::size_t i = 0;
         while( in.size( i + 1 ) >= i + 1 ) {
            const auto c = in.peek_char( i );
            if( ( '0' <= c ) && ( c <= '9' ) ) {
               size <<= 4;
               size |= std::size_t( c - '0' );
               ++i;
               continue;
            }
            if( ( 'a' <= c ) && ( c <= 'f' ) ) {
               size <<= 4;
               size |= std::size_t( c - 'a' + 10 );
               ++i;
               continue;
            }
            if( ( 'A' <= c ) && ( c <= 'F' ) ) {
               size <<= 4;
               size |= std::size_t( c - 'A' + 10 );
               ++i;
               continue;
            }
            break;
         }
         in.bump_in_this_line( i );
         return i > 0;
      }
   };
   // clang-format off

   struct chunk_ext_name : token {};
   struct chunk_ext_val : sor< quoted_string, token > {};
   struct chunk_ext : star_must< one< ';' >, chunk_ext_name, if_must< one< '=' >, chunk_ext_val > > {};

   // clang-format on
   struct chunk_data
   {
      using rule_t = star< abnf::OCTET >::rule_t;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, const std::size_t size, States&&... /*unused*/ )
      {
         if( in.size( size ) >= size ) {
            in.bump( size );
            return true;
         }
         return false;
      }
   };

   namespace internal::chunk_helper
   {
      template< typename Base >
      struct control;

      template< template< typename... > class Control, typename Rule >
      struct control< Control< Rule > >
         : Control< Rule >
      {
         template< apply_mode A,
                   rewind_mode M,
                   template< typename... >
                   class Action,
                   template< typename... >
                   class,
                   typename ParseInput,
                   typename State,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, State&& /*unused*/, States&&... st )
         {
            return Control< Rule >::template match< A, M, Action, Control >( in, st... );
         }
      };

      template< template< typename... > class Control >
      struct control< Control< chunk_size > >
         : remove_first_state< Control< chunk_size > >
      {};

      template< template< typename... > class Control >
      struct control< Control< chunk_data > >
         : remove_first_state< Control< chunk_data > >
      {};

      template< template< typename... > class Control >
      struct bind
      {
         template< typename Rule >
         using type = control< Control< Rule > >;
      };

   } // namespace internal::chunk_helper

   struct chunk
   {
      using impl = seq< chunk_size, chunk_ext, abnf::CRLF, chunk_data, abnf::CRLF >;

      using rule_t = impl::rule_t;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         std::size_t size{};
         return impl::template match< A, M, Action, internal::chunk_helper::bind< Control >::template type >( in, size, st... );
      }
   };

   // clang-format off
   struct last_chunk : seq< plus< one< '0' > >, not_at< digit >, chunk_ext, abnf::CRLF > {};

   struct trailer_part : star< header_field, abnf::CRLF > {};

   struct chunked_body : seq< until< last_chunk, chunk >, trailer_part, abnf::CRLF > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::http

#endif
#line 10 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/if_then.hpp"
       
#line 1 "tao/pegtl/contrib/if_then.hpp"



#ifndef TAO_PEGTL_CONTRIB_IF_THEN_HPP
#define TAO_PEGTL_CONTRIB_IF_THEN_HPP

#include <type_traits>
#line 17 "tao/pegtl/contrib/if_then.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename Cond, typename Then >
      struct if_pair
      {};

      template< typename... Pairs >
      struct if_then;

      template< typename Cond, typename Then, typename... Pairs >
      struct if_then< if_pair< Cond, Then >, Pairs... >
         : if_then_else< Cond, Then, if_then< Pairs... > >
      {
         template< typename ElseCond, typename... Thens >
         using else_if_then = if_then< if_pair< Cond, Then >, Pairs..., if_pair< ElseCond, seq< Thens... > > >;

         template< typename... Thens >
         using else_then = if_then_else< Cond, Then, if_then< Pairs..., if_pair< success, seq< Thens... > > > >;
      };

      template<>
      struct if_then<>
         : failure
      {};

      template< typename... Pairs >
      inline constexpr bool enable_control< if_then< Pairs... > > = false;

   } // namespace internal

   template< typename Cond, typename... Thens >
   struct if_then
      : internal::if_then< internal::if_pair< Cond, internal::seq< Thens... > > >
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 11 "amalgamated.hpp"

#line 1 "tao/pegtl/contrib/json.hpp"
       
#line 1 "tao/pegtl/contrib/json.hpp"



#ifndef TAO_PEGTL_CONTRIB_JSON_HPP
#define TAO_PEGTL_CONTRIB_JSON_HPP






namespace TAO_PEGTL_NAMESPACE::json
{
   // JSON grammar according to RFC 8259

   // clang-format off
   struct ws : one< ' ', '\t', '\n', '\r' > {};

   template< typename R, typename P = ws >
   struct padr : internal::seq< R, internal::star< P > > {};

   struct begin_array : padr< one< '[' > > {};
   struct begin_object : padr< one< '{' > > {};
   struct end_array : one< ']' > {};
   struct end_object : one< '}' > {};
   struct name_separator : pad< one< ':' >, ws > {};
   struct value_separator : padr< one< ',' > > {};

   struct false_ : string< 'f', 'a', 'l', 's', 'e' > {}; // NOLINT(readability-identifier-naming)
   struct null : string< 'n', 'u', 'l', 'l' > {};
   struct true_ : string< 't', 'r', 'u', 'e' > {}; // NOLINT(readability-identifier-naming)

   struct digits : plus< digit > {};
   struct exp : seq< one< 'e', 'E' >, opt< one< '-', '+'> >, must< digits > > {};
   struct frac : if_must< one< '.' >, digits > {};
   struct int_ : sor< one< '0' >, digits > {}; // NOLINT(readability-identifier-naming)
   struct number : seq< opt< one< '-' > >, int_, opt< frac >, opt< exp > > {};

   struct xdigit : pegtl::xdigit {};
   struct unicode : list< seq< one< 'u' >, rep< 4, must< xdigit > > >, one< '\\' > > {};
   struct escaped_char : one< '"', '\\', '/', 'b', 'f', 'n', 'r', 't' > {};
   struct escaped : sor< escaped_char, unicode > {};
   struct unescaped : utf8::range< 0x20, 0x10FFFF > {};
   struct char_ : if_then_else< one< '\\' >, must< escaped >, unescaped > {}; // NOLINT(readability-identifier-naming)

   struct string_content : until< at< one< '"' > >, must< char_ > > {};
   struct string : seq< one< '"' >, must< string_content >, any >
   {
      using content = string_content;
   };

   struct key_content : until< at< one< '"' > >, must< char_ > > {};
   struct key : seq< one< '"' >, must< key_content >, any >
   {
      using content = key_content;
   };

   struct value;

   struct array_element;
   struct array_content : opt< list_must< array_element, value_separator > > {};
   struct array : seq< begin_array, array_content, must< end_array > >
   {
      using begin = begin_array;
      using end = end_array;
      using element = array_element;
      using content = array_content;
   };

   struct member : if_must< key, name_separator, value > {};
   struct object_content : opt< list_must< member, value_separator > > {};
   struct object : seq< begin_object, object_content, must< end_object > >
   {
      using begin = begin_object;
      using end = end_object;
      using element = member;
      using content = object_content;
   };

   struct value : padr< sor< string, number, object, array, false_, true_, null > > {};
   struct array_element : seq< value > {};

   struct text : seq< star< ws >, value > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::json

#endif
#line 13 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/json_pointer.hpp"
       
#line 1 "tao/pegtl/contrib/json_pointer.hpp"



#ifndef TAO_PEGTL_CONTRIB_JSON_POINTER_HPP
#define TAO_PEGTL_CONTRIB_JSON_POINTER_HPP






namespace TAO_PEGTL_NAMESPACE::json_pointer
{
   // JSON pointer grammar according to RFC 6901

   // clang-format off
   struct unescaped : utf8::ranges< 0x0, 0x2E, 0x30, 0x7D, 0x7F, 0x10FFFF > {};
   struct escaped : seq< one< '~' >, one< '0', '1' > > {};

   struct reference_token : star< sor< unescaped, escaped > > {};
   struct json_pointer : star< one< '/' >, reference_token > {};
   // clang-format on

   // relative JSON pointer, see ...

   // clang-format off
   struct non_negative_integer : sor< one< '0' >, plus< digit > > {};
   struct relative_json_pointer : seq< non_negative_integer, sor< one< '#' >, json_pointer > > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::json_pointer

#endif
#line 14 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/parse_tree.hpp"
       
#line 1 "tao/pegtl/contrib/parse_tree.hpp"



#ifndef TAO_PEGTL_CONTRIB_PARSE_TREE_HPP
#define TAO_PEGTL_CONTRIB_PARSE_TREE_HPP

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>
#line 33 "tao/pegtl/contrib/parse_tree.hpp"
namespace TAO_PEGTL_NAMESPACE::parse_tree
{
   template< typename T >
   struct basic_node
   {
      using node_t = T;
      using children_t = std::vector< std::unique_ptr< node_t > >;
      children_t children;

      std::string_view type;
      std::string source;

      TAO_PEGTL_NAMESPACE::internal::iterator m_begin;
      TAO_PEGTL_NAMESPACE::internal::iterator m_end;

      // each node will be default constructed
      basic_node() = default;

      // no copy/move is necessary
      // (nodes are always owned/handled by a std::unique_ptr)
      basic_node( const basic_node& ) = delete;
      basic_node( basic_node&& ) = delete;

      ~basic_node() = default;

      // no assignment either
      basic_node& operator=( const basic_node& ) = delete;
      basic_node& operator=( basic_node&& ) = delete;

      [[nodiscard]] bool is_root() const noexcept
      {
         return type.empty();
      }

      template< typename U >
      [[nodiscard]] bool is_type() const noexcept
      {
         return type == demangle< U >();
      }

      template< typename U >
      void set_type() noexcept
      {
         type = demangle< U >();
      }

      [[nodiscard]] position begin() const
      {
         return position( m_begin, source );
      }

      [[nodiscard]] position end() const
      {
         return position( m_end, source );
      }

      [[nodiscard]] bool has_content() const noexcept
      {
         return m_end.data != nullptr;
      }

      [[nodiscard]] std::string_view string_view() const noexcept
      {
         assert( has_content() );
         return std::string_view( m_begin.data, m_end.data - m_begin.data );
      }

      [[nodiscard]] std::string string() const
      {
         assert( has_content() );
         return std::string( m_begin.data, m_end.data );
      }

      template< tracking_mode P = tracking_mode::eager, typename Eol = eol::lf_crlf >
      [[nodiscard]] memory_input< P, Eol > as_memory_input() const
      {
         assert( has_content() );
         return { m_begin.data, m_end.data, source, m_begin.byte, m_begin.line, m_begin.column };
      }

      template< typename... States >
      void remove_content( States&&... /*unused*/ ) noexcept
      {
         m_end = TAO_PEGTL_NAMESPACE::internal::iterator();
      }

      // all non-root nodes are initialized by calling this method
      template< typename Rule, typename ParseInput, typename... States >
      void start( const ParseInput& in, States&&... /*unused*/ )
      {
         set_type< Rule >();
         source = in.source();
         m_begin = TAO_PEGTL_NAMESPACE::internal::iterator( in.iterator() );
      }

      // if parsing of the rule succeeded, this method is called
      template< typename Rule, typename ParseInput, typename... States >
      void success( const ParseInput& in, States&&... /*unused*/ ) noexcept
      {
         m_end = TAO_PEGTL_NAMESPACE::internal::iterator( in.iterator() );
      }

      // if parsing of the rule failed, this method is called
      template< typename Rule, typename ParseInput, typename... States >
      void failure( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      // if parsing of the rule failed with an exception, this method is called
      template< typename Rule, typename ParseInput, typename... States >
      void unwind( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      // if parsing succeeded and the (optional) transform call
      // did not discard the node, it is appended to its parent.
      // note that "child" is the node whose Rule just succeeded
      // and "*this" is the parent where the node should be appended.
      template< typename... States >
      void emplace_back( std::unique_ptr< node_t >&& child, States&&... /*unused*/ )
      {
         assert( child );
         children.emplace_back( std::move( child ) );
      }
   };

   struct node
      : basic_node< node >
   {};

   namespace internal
   {
      template< typename Node >
      struct state
      {
         std::vector< std::unique_ptr< Node > > stack;

         state()
         {
            emplace_back();
         }

         void emplace_back()
         {
            stack.emplace_back( std::make_unique< Node >() );
         }

         [[nodiscard]] std::unique_ptr< Node >& back() noexcept
         {
            assert( !stack.empty() );
            return stack.back();
         }

         void pop_back() noexcept
         {
            assert( !stack.empty() );
            return stack.pop_back();
         }
      };

      template< typename Selector, typename... Parameters >
      void transform( Parameters&&... /*unused*/ ) noexcept
      {}

      template< typename Selector, typename ParseInput, typename Node, typename... States >
      auto transform( const ParseInput& in, std::unique_ptr< Node >& n, States&&... st ) noexcept( noexcept( Selector::transform( in, n, st... ) ) )
         -> decltype( (void)Selector::transform( in, n, st... ) )
      {
         Selector::transform( in, n, st... );
      }

      template< typename Selector, typename ParseInput, typename Node, typename... States >
      auto transform( const ParseInput& /*unused*/, std::unique_ptr< Node >& n, States&&... st ) noexcept( noexcept( Selector::transform( n, st... ) ) )
         -> decltype( (void)Selector::transform( n, st... ) )
      {
         Selector::transform( n, st... );
      }

      template< typename Rule, template< typename... > class Selector >
      inline constexpr bool is_selected_node = ( TAO_PEGTL_NAMESPACE::internal::enable_control< Rule > && Selector< Rule >::value );

      template< unsigned Level, typename Subs, template< typename... > class Selector >
      inline constexpr bool is_leaf{};

      template< typename... Rules, template< typename... > class Selector >
      inline constexpr bool is_leaf< 0, type_list< Rules... >, Selector > = ( sizeof...( Rules ) == 0 );

      template< unsigned Level, typename Rule, template< typename... > class Selector >
      inline constexpr bool is_unselected_branch = ( !is_selected_node< Rule, Selector > && is_leaf< Level, typename Rule::subs_t, Selector > );

      template< unsigned Level, typename... Rules, template< typename... > class Selector >
      inline constexpr bool is_leaf< Level, type_list< Rules... >, Selector > = ( is_unselected_branch< Level - 1, Rules, Selector > && ... );

      template< typename Node, template< typename... > class Selector, template< typename... > class Control >
      struct make_control
      {
         template< typename Rule, bool, bool >
         struct state_handler;

         template< typename Rule >
         using type = rotate_states_right< state_handler< Rule, is_selected_node< Rule, Selector >, is_leaf< 8, typename Rule::subs_t, Selector > > >;
      };

      template< typename Node, template< typename... > class Selector, template< typename... > class Control >
      template< typename Rule >
      struct make_control< Node, Selector, Control >::state_handler< Rule, false, true >
         : remove_first_state< Control< Rule > >
      {};

      template< typename Node, template< typename... > class Selector, template< typename... > class Control >
      template< typename Rule >
      struct make_control< Node, Selector, Control >::state_handler< Rule, false, false >
         : remove_first_state< Control< Rule > >
      {
         template< typename ParseInput, typename... States >
         static void start( const ParseInput& /*unused*/, state< Node >& state, States&&... /*unused*/ )
         {
            state.emplace_back();
         }

         template< typename ParseInput, typename... States >
         static void success( const ParseInput& /*unused*/, state< Node >& state, States&&... /*unused*/ )
         {
            auto n = std::move( state.back() );
            state.pop_back();
            for( auto& c : n->children ) {
               state.back()->children.emplace_back( std::move( c ) );
            }
         }

         template< typename ParseInput, typename... States >
         static void failure( const ParseInput& /*unused*/, state< Node >& state, States&&... /*unused*/ )
         {
            state.pop_back();
         }

         template< typename ParseInput, typename... States >
         static void unwind( const ParseInput& /*unused*/, state< Node >& state, States&&... /*unused*/ )
         {
            state.pop_back();
         }
      };

      template< typename Node, template< typename... > class Selector, template< typename... > class Control >
      template< typename Rule, bool B >
      struct make_control< Node, Selector, Control >::state_handler< Rule, true, B >
         : remove_first_state< Control< Rule > >
      {
         template< typename ParseInput, typename... States >
         static void start( const ParseInput& in, state< Node >& state, States&&... st )
         {
            Control< Rule >::start( in, st... );
            state.emplace_back();
            state.back()->template start< Rule >( in, st... );
         }

         template< typename ParseInput, typename... States >
         static void success( const ParseInput& in, state< Node >& state, States&&... st )
         {
            auto n = std::move( state.back() );
            state.pop_back();
            n->template success< Rule >( in, st... );
            transform< Selector< Rule > >( in, n, st... );
            if( n ) {
               state.back()->emplace_back( std::move( n ), st... );
            }
            Control< Rule >::success( in, st... );
         }

         template< typename ParseInput, typename... States >
         static void failure( const ParseInput& in, state< Node >& state, States&&... st )
         {
            state.back()->template failure< Rule >( in, st... );
            state.pop_back();
            Control< Rule >::failure( in, st... );
         }

         template< typename ParseInput, typename... States >
         static void unwind( const ParseInput& in, state< Node >& state, States&&... st )
         {
            state.back()->template unwind< Rule >( in, st... );
            state.pop_back();
            if constexpr( TAO_PEGTL_NAMESPACE::internal::has_unwind< Control< Rule >, void, const ParseInput&, States... > ) {
               Control< Rule >::unwind( in, st... );
            }
         }
      };

      template< typename >
      using store_all = std::true_type;

      template< typename >
      struct selector;

      template< typename T >
      struct selector< std::tuple< T > >
      {
         using type = typename T::type;
      };

      template< typename... Ts >
      struct selector< std::tuple< Ts... > >
      {
         static_assert( sizeof...( Ts ) == 0, "multiple matches found" );
         using type = std::false_type;
      };

      template< typename T >
      using selector_t = typename selector< T >::type;

      template< typename Rule, typename Collection >
      using select_tuple = std::conditional_t< Collection::template contains< Rule >, std::tuple< Collection >, std::tuple<> >;

   } // namespace internal

   template< typename Rule, typename... Collections >
   using selector = internal::selector_t< decltype( std::tuple_cat( std::declval< internal::select_tuple< Rule, Collections > >()... ) ) >;

   template< typename Base >
   struct apply
      : std::true_type
   {
      template< typename... Rules >
      struct on
      {
         using type = Base;

         template< typename Rule >
         static constexpr bool contains = ( std::is_same_v< Rule, Rules > || ... );
      };
   };

   struct store_content
      : apply< store_content >
   {};

   // some nodes don't need to store their content
   struct remove_content
      : apply< remove_content >
   {
      template< typename Node, typename... States >
      static void transform( std::unique_ptr< Node >& n, States&&... st ) noexcept( noexcept( n->Node::remove_content( st... ) ) )
      {
         n->remove_content( st... );
      }
   };

   // if a node has only one child, replace the node with its child, otherwise remove content
   struct fold_one
      : apply< fold_one >
   {
      template< typename Node, typename... States >
      static void transform( std::unique_ptr< Node >& n, States&&... st ) noexcept( noexcept( n->children.size(), n->Node::remove_content( st... ) ) )
      {
         if( n->children.size() == 1 ) {
            n = std::move( n->children.front() );
         }
         else {
            n->remove_content( st... );
         }
      }
   };

   // if a node has no children, discard the node, otherwise remove content
   struct discard_empty
      : apply< discard_empty >
   {
      template< typename Node, typename... States >
      static void transform( std::unique_ptr< Node >& n, States&&... st ) noexcept( noexcept( n->children.empty(), n->Node::remove_content( st... ) ) )
      {
         if( n->children.empty() ) {
            n.reset();
         }
         else {
            n->remove_content( st... );
         }
      }
   };

   template< typename Rule,
             typename Node,
             template< typename... > class Selector = internal::store_all,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   [[nodiscard]] std::unique_ptr< Node > parse( ParseInput&& in, States&&... st )
   {
      internal::state< Node > state;
      if( !TAO_PEGTL_NAMESPACE::parse< Rule, Action, internal::make_control< Node, Selector, Control >::template type >( in, st..., state ) ) {
         return nullptr;
      }
      assert( state.stack.size() == 1 );
      return std::move( state.back() );
   }

   template< typename Rule,
             template< typename... > class Selector = internal::store_all,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   [[nodiscard]] std::unique_ptr< node > parse( ParseInput&& in, States&&... st )
   {
      return parse< Rule, node, Selector, Action, Control >( in, st... );
   }

} // namespace TAO_PEGTL_NAMESPACE::parse_tree

#endif
#line 15 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/parse_tree_to_dot.hpp"
       
#line 1 "tao/pegtl/contrib/parse_tree_to_dot.hpp"



#ifndef TAO_PEGTL_CONTRIB_PARSE_TREE_TO_DOT_HPP
#define TAO_PEGTL_CONTRIB_PARSE_TREE_TO_DOT_HPP

#include <cassert>
#include <ostream>
#include <string>



namespace TAO_PEGTL_NAMESPACE::parse_tree
{
   namespace internal
   {
      inline void escape( std::ostream& os, const std::string_view s )
      {
         static const char* h = "0123456789abcdef";

         const char* p = s.data();
         const char* l = p;
         const char* const e = s.data() + s.size();
         while( p != e ) {
            const unsigned char c = *p;
            if( c == '\\' ) {
               os.write( l, p - l );
               l = ++p;
               os << "\\\\\\\\";
            }
            else if( c == '"' ) {
               os.write( l, p - l );
               l = ++p;
               os << "\\\\\\\"";
            }
            else if( c < 32 ) {
               os.write( l, p - l );
               l = ++p;
               switch( c ) {
                  case '\b':
                     os << "\\\\b";
                     break;
                  case '\f':
                     os << "\\\\f";
                     break;
                  case '\n':
                     os << "\\\\n";
                     break;
                  case '\r':
                     os << "\\\\r";
                     break;
                  case '\t':
                     os << "\\\\t";
                     break;
                  default:
                     os << "\\\\u00" << h[ ( c & 0xf0 ) >> 4 ] << h[ c & 0x0f ];
               }
            }
            else if( c == 127 ) {
               os.write( l, p - l );
               l = ++p;
               os << "\\\\u007f";
            }
            else {
               ++p;
            }
         }
         os.write( l, p - l );
      }

      template< typename Node >
      void print_dot_node( std::ostream& os, const Node& n, const std::string_view s )
      {
         os << "  x" << &n << " [ label=\"";
         escape( os, s );
         if( n.has_content() ) {
            os << "\\n\\\"";
            escape( os, n.string_view() );
            os << "\\\"";
         }
         os << "\" ]\n";
         if( !n.children.empty() ) {
            os << "  x" << &n << " -> { ";
            for( auto& up : n.children ) {
               os << "x" << up.get() << ( ( up == n.children.back() ) ? " }\n" : ", " );
            }
            for( auto& up : n.children ) {
               print_dot_node( os, *up, up->type );
            }
         }
      }

   } // namespace internal

   template< typename Node >
   void print_dot( std::ostream& os, const Node& n )
   {
      os << "digraph parse_tree\n{\n";
      internal::print_dot_node( os, n, n.is_root() ? "ROOT" : n.type );
      os << "}\n";
   }

} // namespace TAO_PEGTL_NAMESPACE::parse_tree

#endif
#line 16 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/print.hpp"
       
#line 1 "tao/pegtl/contrib/print.hpp"



#ifndef TAO_PEGTL_CONTRIB_PRINT_HPP
#define TAO_PEGTL_CONTRIB_PRINT_HPP

#include <ostream>






namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename Name >
      struct print_names
      {
         static void visit( std::ostream& os )
         {
            os << demangle< Name >() << '\n';
         }
      };

      template< typename Name >
      struct print_debug
      {
         static void visit( std::ostream& os )
         {
            const auto first = demangle< Name >();
            os << first << '\n';

            const auto second = demangle< typename Name::rule_t >();
            if( first != second ) {
               os << " (aka) " << second << '\n';
            }

            print_subs( os, typename Name::subs_t() );

            os << '\n';
         }

      private:
         template< typename... Rules >
         static void print_subs( std::ostream& os, type_list< Rules... > /*unused*/ )
         {
            ( print_sub< Rules >( os ), ... );
         }

         template< typename Rule >
         static void print_sub( std::ostream& os )
         {
            os << " (sub) " << demangle< Rule >() << '\n';
         }
      };

   } // namespace internal

   template< typename Grammar >
   void print_names( std::ostream& os )
   {
      visit< Grammar, internal::print_names >( os );
   }

   template< typename Grammar >
   void print_debug( std::ostream& os )
   {
      visit< Grammar, internal::print_debug >( os );
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 17 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/print_coverage.hpp"
       
#line 1 "tao/pegtl/contrib/print_coverage.hpp"



#ifndef TAO_PEGTL_CONTRIB_PRINT_COVERAGE_HPP
#define TAO_PEGTL_CONTRIB_PRINT_COVERAGE_HPP

#include <ostream>



namespace TAO_PEGTL_NAMESPACE
{
   inline std::ostream& operator<<( std::ostream& os, const coverage_result& result )
   {
      os << "[\n";
      bool f = true;
      for( const auto& [ k, v ] : result ) {
         if( f ) {
            f = false;
         }
         else {
            os << ",\n";
         }
         os << "  {\n"
            << "    \"rule\": \"" << k << "\",\n"
            << "    \"start\": " << v.start << ", \"success\": " << v.success << ", \"failure\": " << v.failure << ", \"unwind\": " << v.unwind << ", \"raise\": " << v.raise << ",\n";
         if( v.branches.empty() ) {
            os << "    \"branches\": []\n";
         }
         else {
            os << "    \"branches\": [\n";
            bool f2 = true;
            for( const auto& [ k2, v2 ] : v.branches ) {
               if( f2 ) {
                  f2 = false;
               }
               else {
                  os << ",\n";
               }
               os << "      { \"branch\": \"" << k2 << "\", \"start\": " << v2.start << ", \"success\": " << v2.success << ", \"failure\": " << v2.failure << ", \"unwind\": " << v2.unwind << ", \"raise\": " << v2.raise << " }";
            }
            os << "\n    ]\n";
         }
         os << "  }";
      }
      os << "\n";
      os << "]\n";
      return os;
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 18 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/raw_string.hpp"
       
#line 1 "tao/pegtl/contrib/raw_string.hpp"



#ifndef TAO_PEGTL_CONTRIB_RAW_STRING_HPP
#define TAO_PEGTL_CONTRIB_RAW_STRING_HPP

#include <cstddef>
#include <type_traits>
#line 18 "tao/pegtl/contrib/raw_string.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< char Open, char Marker >
      struct raw_string_open
      {
         using rule_t = raw_string_open;
         using subs_t = empty_list;

         template< apply_mode A,
                   rewind_mode,
                   template< typename... >
                   class Action,
                   template< typename... >
                   class Control,
                   typename ParseInput,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, std::size_t& marker_size, States&&... /*unused*/ ) noexcept( noexcept( in.size( 0 ) ) )
         {
            if( in.empty() || ( in.peek_char( 0 ) != Open ) ) {
               return false;
            }
            for( std::size_t i = 1; i < in.size( i + 1 ); ++i ) {
               switch( const auto c = in.peek_char( i ) ) {
                  case Open:
                     marker_size = i + 1;
                     in.bump_in_this_line( marker_size );
                     (void)eol::match( in );
                     return true;
                  case Marker:
                     break;
                  default:
                     return false;
               }
            }
            return false;
         }
      };

      template< char Open, char Marker >
      inline constexpr bool enable_control< raw_string_open< Open, Marker > > = false;

      template< char Marker, char Close >
      struct at_raw_string_close
      {
         using rule_t = at_raw_string_close;
         using subs_t = empty_list;

         template< apply_mode A,
                   rewind_mode,
                   template< typename... >
                   class Action,
                   template< typename... >
                   class Control,
                   typename ParseInput,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, const std::size_t& marker_size, States&&... /*unused*/ ) noexcept( noexcept( in.size( 0 ) ) )
         {
            if( in.size( marker_size ) < marker_size ) {
               return false;
            }
            if( in.peek_char( 0 ) != Close ) {
               return false;
            }
            if( in.peek_char( marker_size - 1 ) != Close ) {
               return false;
            }
            for( std::size_t i = 0; i < ( marker_size - 2 ); ++i ) {
               if( in.peek_char( i + 1 ) != Marker ) {
                  return false;
               }
            }
            return true;
         }
      };

      template< char Marker, char Close >
      inline constexpr bool enable_control< at_raw_string_close< Marker, Close > > = false;

      template< typename Cond, typename... Rules >
      struct raw_string_until
         : raw_string_until< Cond, seq< Rules... > >
      {};

      template< typename Cond >
      struct raw_string_until< Cond >
      {
         using rule_t = raw_string_until;
         using subs_t = type_list< Cond >;

         template< apply_mode A,
                   rewind_mode M,
                   template< typename... >
                   class Action,
                   template< typename... >
                   class Control,
                   typename ParseInput,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, const std::size_t& marker_size, States&&... st )
         {
            auto m = in.template mark< M >();

            while( !Control< Cond >::template match< A, rewind_mode::required, Action, Control >( in, marker_size, st... ) ) {
               if( in.empty() ) {
                  return false;
               }
               in.bump();
            }
            return m( true );
         }
      };

      template< typename Cond, typename Rule >
      struct raw_string_until< Cond, Rule >
      {
         using rule_t = raw_string_until;
         using subs_t = type_list< Cond, Rule >;

         template< apply_mode A,
                   rewind_mode M,
                   template< typename... >
                   class Action,
                   template< typename... >
                   class Control,
                   typename ParseInput,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, const std::size_t& marker_size, States&&... st )
         {
            auto m = in.template mark< M >();
            using m_t = decltype( m );

            while( !Control< Cond >::template match< A, rewind_mode::required, Action, Control >( in, marker_size, st... ) ) {
               if( !Control< Rule >::template match< A, m_t::next_rewind_mode, Action, Control >( in, st... ) ) {
                  return false;
               }
            }
            return m( true );
         }
      };

      template< typename Cond, typename... Rules >
      inline constexpr bool enable_control< raw_string_until< Cond, Rules... > > = false;

   } // namespace internal

   // raw_string matches Lua-style long literals.
   //
   // The following description was taken from the Lua documentation
   // (see http://www.lua.org/docs.html):
   //
   // - An "opening long bracket of level n" is defined as an opening square
   //   bracket followed by n equal signs followed by another opening square
   //   bracket. So, an opening long bracket of level 0 is written as `[[`,
   //   an opening long bracket of level 1 is written as `[=[`, and so on.
   // - A "closing long bracket" is defined similarly; for instance, a closing
   //   long bracket of level 4 is written as `]====]`.
   // - A "long literal" starts with an opening long bracket of any level and
   //   ends at the first closing long bracket of the same level. It can
   //   contain any text except a closing bracket of the same level.
   // - Literals in this bracketed form can run for several lines, do not
   //   interpret any escape sequences, and ignore long brackets of any other
   //   level.
   // - For convenience, when the opening long bracket is eagerly followed
   //   by a newline, the newline is not included in the string.
   //
   // Note that unlike Lua's long literal, a raw_string is customizable to use
   // other characters than `[`, `=` and `]` for matching. Also note that Lua
   // introduced newline-specific replacements in Lua 5.2, which we do not
   // support on the grammar level.

   template< char Open, char Marker, char Close, typename... Contents >
   struct raw_string
   {
      // This is used for binding the apply()-method and for error-reporting
      // when a raw string is not closed properly or has invalid content.
      struct content
         : internal::raw_string_until< internal::at_raw_string_close< Marker, Close >, Contents... >
      {};

      using rule_t = raw_string;
      using subs_t = empty_list; // type_list< internal::raw_string_open< Open, Marker >, internal::must< content > >;

      template< apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         std::size_t marker_size;
         if( Control< internal::raw_string_open< Open, Marker > >::template match< A, M, Action, Control >( in, marker_size, st... ) ) {
            // TODO: Do not rely on must<>
            (void)Control< internal::must< content > >::template match< A, M, Action, Control >( in, marker_size, st... );
            in.bump_in_this_line( marker_size );
            return true;
         }
         return false;
      }
   };

   template< typename Name, char Open, char Marker, char Close >
   struct analyze_traits< Name, raw_string< Open, Marker, Close > >
      : analyze_any_traits<>
   {};

   template< typename Name, char Open, char Marker, char Close, typename... Contents >
   struct analyze_traits< Name, raw_string< Open, Marker, Close, Contents... > >
      : analyze_traits< Name, typename seq< any, star< Contents... >, any >::rule_t >
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 19 "amalgamated.hpp"

#line 1 "tao/pegtl/contrib/remove_last_states.hpp"
       
#line 1 "tao/pegtl/contrib/remove_last_states.hpp"



#ifndef TAO_PEGTL_CONTRIB_REMOVE_LAST_STATES_HPP
#define TAO_PEGTL_CONTRIB_REMOVE_LAST_STATES_HPP

#include <tuple>
#include <utility>





namespace TAO_PEGTL_NAMESPACE
{
   // Remove the last N states of start(), success(), failure(), raise(), apply(), and apply0()
   template< typename Base, std::size_t N >
   struct remove_last_states
      : Base
   {
      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void start_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::start( in, std::get< Is >( t )... ) ) )
      {
         Base::start( in, std::get< Is >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& in, States&&... st ) noexcept( noexcept( start_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) ) )
      {
         start_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void success_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::success( in, std::get< Is >( t )... ) ) )
      {
         Base::success( in, std::get< Is >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& in, States&&... st ) noexcept( noexcept( success_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) ) )
      {
         success_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void failure_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::failure( in, std::get< Is >( t )... ) ) )
      {
         Base::failure( in, std::get< Is >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& in, States&&... st ) noexcept( noexcept( failure_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) ) )
      {
         failure_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      [[noreturn]] static void raise_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
      {
         Base::raise( in, std::get< Is >( t )... );
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... st )
      {
         raise_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static auto unwind_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
         -> std::enable_if_t< internal::has_unwind< Base, void, const ParseInput&, std::tuple_element_t< Is, Tuple >... > >
      {
         Base::unwind( in, std::get< Is >( t )... );
      }

      template< typename ParseInput, typename... States >
      static auto unwind( const ParseInput& in, States&&... st )
         -> decltype( unwind_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) )
      {
         unwind_impl( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename Tuple, std::size_t... Is >
      static auto apply_impl( const Iterator& begin, const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply< Action >( begin, in, std::get< Is >( t )... ) ) )
         -> decltype( Base::template apply< Action >( begin, in, std::get< Is >( t )... ) )
      {
         return Base::template apply< Action >( begin, in, std::get< Is >( t )... );
      }

      template< template< typename... > class Action, typename Iterator, typename ParseInput, typename... States >
      static auto apply( const Iterator& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) ) )
         -> decltype( apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) )
      {
         return apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }

      template< template< typename... > class Action, typename ParseInput, typename Tuple, std::size_t... Is >
      static auto apply0_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply0< Action >( in, std::get< Is >( t )... ) ) )
         -> decltype( Base::template apply0< Action >( in, std::get< Is >( t )... ) )
      {
         return Base::template apply0< Action >( in, std::get< Is >( t )... );
      }

      template< template< typename... > class Action, typename ParseInput, typename... States >
      static auto apply0( const ParseInput& in, States&&... st ) noexcept( noexcept( apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) ) )
         -> decltype( apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() ) )
      {
         return apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< sizeof...( st ) - N >() );
      }
   };

   template< typename Base >
   using remove_last_state = remove_last_states< Base, 1 >;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 21 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/rep_one_min_max.hpp"
       
#line 1 "tao/pegtl/contrib/rep_one_min_max.hpp"



#ifndef TAO_PEGTL_CONTRIB_REP_ONE_MIN_MAX_HPP
#define TAO_PEGTL_CONTRIB_REP_ONE_MIN_MAX_HPP

#include <algorithm>
#include <type_traits>
#line 20 "tao/pegtl/contrib/rep_one_min_max.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< unsigned Min, unsigned Max, char C >
      struct rep_one_min_max
      {
         using rule_t = rep_one_min_max;
         using subs_t = empty_list;

         static_assert( Min <= Max );

         template< typename ParseInput >
         [[nodiscard]] static bool match( ParseInput& in )
         {
            const auto size = in.size( Max + 1 );
            if( size < Min ) {
               return false;
            }
            std::size_t i = 0;
            while( ( i < size ) && ( in.peek_char( i ) == C ) ) {
               ++i;
            }
            if( ( Min <= i ) && ( i <= Max ) ) {
               bump_help< result_on_found::success, ParseInput, char, C >( in, i );
               return true;
            }
            return false;
         }
      };

      template< unsigned Max, char C >
      struct rep_one_min_max< 0, Max, C >
      {
         using rule_t = rep_one_min_max;
         using subs_t = empty_list;

         template< typename ParseInput >
         [[nodiscard]] static bool match( ParseInput& in )
         {
            const auto size = in.size( Max + 1 );
            std::size_t i = 0;
            while( ( i < size ) && ( in.peek_char( i ) == C ) ) {
               ++i;
            }
            if( i <= Max ) {
               bump_help< result_on_found::success, ParseInput, char, C >( in, i );
               return true;
            }
            return false;
         }
      };

      template< unsigned Min, unsigned Max, char C >
      inline constexpr bool enable_control< rep_one_min_max< Min, Max, C > > = false;

   } // namespace internal

   inline namespace ascii
   {
      template< unsigned Min, unsigned Max, char C >
      struct rep_one_min_max
         : internal::rep_one_min_max< Min, Max, C >
      {};

   } // namespace ascii

   template< typename Name, unsigned Min, unsigned Max, char C >
   struct analyze_traits< Name, internal::rep_one_min_max< Min, Max, C > >
      : std::conditional_t< ( Min != 0 ), analyze_any_traits<>, analyze_opt_traits<> >
   {};

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 22 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/rep_string.hpp"
       
#line 1 "tao/pegtl/contrib/rep_string.hpp"



#ifndef TAO_PEGTL_CONTRIB_REP_STRING_HPP
#define TAO_PEGTL_CONTRIB_REP_STRING_HPP

#include <cstddef>




namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< std::size_t, typename, char... >
      struct make_rep_string;

      template< char... Ss, char... Cs >
      struct make_rep_string< 0, string< Ss... >, Cs... >
      {
         using type = string< Ss... >;
      };

      template< std::size_t N, char... Ss, char... Cs >
      struct make_rep_string< N, string< Ss... >, Cs... >
         : make_rep_string< N - 1, string< Ss..., Cs... >, Cs... >
      {};

   } // namespace internal

   inline namespace ascii
   {
      template< std::size_t N, char... Cs >
      struct rep_string
         : internal::make_rep_string< N, internal::string<>, Cs... >::type
      {};

   } // namespace ascii

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 23 "amalgamated.hpp"


#line 1 "tao/pegtl/contrib/to_string.hpp"
       
#line 1 "tao/pegtl/contrib/to_string.hpp"



#ifndef TAO_PEGTL_CONTRIB_TO_STRING_HPP
#define TAO_PEGTL_CONTRIB_TO_STRING_HPP

#include <string>



namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< typename >
      struct to_string;

      template< template< char... > class X, char... Cs >
      struct to_string< X< Cs... > >
      {
         [[nodiscard]] static std::string get()
         {
            const char s[] = { Cs..., 0 };
            return std::string( s, sizeof...( Cs ) );
         }
      };

   } // namespace internal

   template< typename T >
   [[nodiscard]] std::string to_string()
   {
      return internal::to_string< T >::get();
   }

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 26 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/trace.hpp"
       
#line 1 "tao/pegtl/contrib/trace.hpp"



#ifndef TAO_PEGTL_CONTRIB_TRACE_HPP
#define TAO_PEGTL_CONTRIB_TRACE_HPP

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <tuple>
#line 23 "tao/pegtl/contrib/trace.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   template< bool HideInternal = false, bool UseColor = true, std::size_t IndentIncrement = 2, std::size_t InitialIndent = 8 >
   struct tracer_traits
   {
      template< typename Rule >
      static constexpr bool enable = ( HideInternal ? normal< Rule >::enable : true );

      static constexpr std::size_t initial_indent = InitialIndent;
      static constexpr std::size_t indent_increment = IndentIncrement;

      static constexpr std::string_view ansi_reset = UseColor ? "\033[m" : "";
      static constexpr std::string_view ansi_rule = UseColor ? "\033[36m" : "";
      static constexpr std::string_view ansi_hide = UseColor ? "\033[37m" : "";

      static constexpr std::string_view ansi_position = UseColor ? "\033[1;34m" : "";
      static constexpr std::string_view ansi_success = UseColor ? "\033[32m" : "";
      static constexpr std::string_view ansi_failure = UseColor ? "\033[31m" : "";
      static constexpr std::string_view ansi_raise = UseColor ? "\033[1;31m" : "";
      static constexpr std::string_view ansi_unwind = UseColor ? "\033[31m" : "";
      static constexpr std::string_view ansi_apply = UseColor ? "\033[1;36m" : "";
   };

   using standard_tracer_traits = tracer_traits< true >;
   using complete_tracer_traits = tracer_traits< false >;

   template< typename TracerTraits >
   struct tracer
   {
      const std::ios_base::fmtflags m_flags;
      std::size_t m_count = 0;
      std::vector< std::size_t > m_stack;
      position m_position;

      template< typename Rule >
      static constexpr bool enable = TracerTraits::template enable< Rule >;

      template< typename ParseInput >
      explicit tracer( const ParseInput& in )
         : m_flags( std::cerr.flags() ),
           m_position( in.position() )
      {
         std::cerr << std::left;
         print_position();
      }

      tracer( const tracer& ) = delete;
      tracer( tracer&& ) = delete;

      ~tracer()
      {
         std::cerr.flags( m_flags );
      }

      tracer& operator=( const tracer& ) = delete;
      tracer& operator=( tracer&& ) = delete;

      [[nodiscard]] std::size_t indent() const noexcept
      {
         return TracerTraits::initial_indent + TracerTraits::indent_increment * m_stack.size();
      }

      void print_position() const
      {
         std::cerr << std::setw( indent() ) << ' ' << TracerTraits::ansi_position << "position" << TracerTraits::ansi_reset << ' ' << m_position << '\n';
      }

      void update_position( const position& p )
      {
         if( m_position != p ) {
            m_position = p;
            print_position();
         }
      }

      template< typename Rule, typename ParseInput, typename... States >
      void start( const ParseInput& /*unused*/, States&&... /*unused*/ )
      {
         std::cerr << '#' << std::setw( indent() - 1 ) << ++m_count << TracerTraits::ansi_rule << demangle< Rule >() << TracerTraits::ansi_reset << '\n';
         m_stack.push_back( m_count );
      }

      template< typename Rule, typename ParseInput, typename... States >
      void success( const ParseInput& in, States&&... /*unused*/ )
      {
         const auto prev = m_stack.back();
         m_stack.pop_back();
         std::cerr << std::setw( indent() ) << ' ' << TracerTraits::ansi_success << "success" << TracerTraits::ansi_reset;
         if( m_count != prev ) {
            std::cerr << " #" << prev << ' ' << TracerTraits::ansi_hide << demangle< Rule >() << TracerTraits::ansi_reset;
         }
         std::cerr << '\n';
         update_position( in.position() );
      }

      template< typename Rule, typename ParseInput, typename... States >
      void failure( const ParseInput& in, States&&... /*unused*/ )
      {
         const auto prev = m_stack.back();
         m_stack.pop_back();
         std::cerr << std::setw( indent() ) << ' ' << TracerTraits::ansi_failure << "failure" << TracerTraits::ansi_reset;
         if( m_count != prev ) {
            std::cerr << " #" << prev << ' ' << TracerTraits::ansi_hide << demangle< Rule >() << TracerTraits::ansi_reset;
         }
         std::cerr << '\n';
         update_position( in.position() );
      }

      template< typename Rule, typename ParseInput, typename... States >
      void raise( const ParseInput& /*unused*/, States&&... /*unused*/ )
      {
         std::cerr << std::setw( indent() ) << ' ' << TracerTraits::ansi_raise << "raise" << TracerTraits::ansi_reset << ' ' << TracerTraits::ansi_rule << demangle< Rule >() << TracerTraits::ansi_reset << '\n';
      }

      template< typename Rule, typename ParseInput, typename... States >
      void unwind( const ParseInput& in, States&&... /*unused*/ )
      {
         const auto prev = m_stack.back();
         m_stack.pop_back();
         std::cerr << std::setw( indent() ) << ' ' << TracerTraits::ansi_unwind << "unwind" << TracerTraits::ansi_reset;
         if( m_count != prev ) {
            std::cerr << " #" << prev << ' ' << TracerTraits::ansi_hide << demangle< Rule >() << TracerTraits::ansi_reset;
         }
         std::cerr << '\n';
         update_position( in.position() );
      }

      template< typename Rule, typename ParseInput, typename... States >
      void apply( const ParseInput& /*unused*/, States&&... /*unused*/ )
      {
         std::cerr << std::setw( static_cast< int >( indent() - TracerTraits::indent_increment ) ) << ' ' << TracerTraits::ansi_apply << "apply" << TracerTraits::ansi_reset << '\n';
      }

      template< typename Rule, typename ParseInput, typename... States >
      void apply0( const ParseInput& /*unused*/, States&&... /*unused*/ )
      {
         std::cerr << std::setw( static_cast< int >( indent() - TracerTraits::indent_increment ) ) << ' ' << TracerTraits::ansi_apply << "apply0" << TracerTraits::ansi_reset << '\n';
      }

      template< typename Rule,
                template< typename... > class Action = nothing,
                template< typename... > class Control = normal,
                typename ParseInput,
                typename... States >
      bool parse( ParseInput&& in, States&&... st )
      {
         return TAO_PEGTL_NAMESPACE::parse< Rule, Action, state_control< Control >::template type >( in, st..., *this );
      }
   };

   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   bool standard_trace( ParseInput&& in, States&&... st )
   {
      tracer< standard_tracer_traits > tr( in );
      return tr.parse< Rule, Action, Control >( in, st... );
   }

   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   bool complete_trace( ParseInput&& in, States&&... st )
   {
      tracer< complete_tracer_traits > tr( in );
      return tr.parse< Rule, Action, Control >( in, st... );
   }

   template< typename Tracer >
   struct trace
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( sizeof...( st ) == 0 ) {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, state_control< Control >::template type >( in, st..., Tracer( in ) );
         }
         else if constexpr( !std::is_same_v< std::tuple_element_t< sizeof...( st ) - 1, std::tuple< States... > >, Tracer& > ) {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, state_control< Control >::template type >( in, st..., Tracer( in ) );
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
   };

   using trace_standard = trace< tracer< standard_tracer_traits > >;
   using trace_complete = trace< tracer< complete_tracer_traits > >;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 27 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/uint16.hpp"
       
#line 1 "tao/pegtl/contrib/uint16.hpp"



#ifndef TAO_PEGTL_CONTRIB_UINT16_HPP
#define TAO_PEGTL_CONTRIB_UINT16_HPP





#line 1 "tao/pegtl/contrib/internal/peek_mask_uint.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_mask_uint.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_MASK_UINT_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_MASK_UINT_HPP

#include <cstddef>
#include <cstdint>




#line 1 "tao/pegtl/contrib/internal/read_uint.hpp"
       
#line 1 "tao/pegtl/contrib/internal/read_uint.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_READ_UINT_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_READ_UINT_HPP

#include <cstdint>



#line 1 "tao/pegtl/contrib/internal/endian.hpp"
       
#line 1 "tao/pegtl/contrib/internal/endian.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_HPP

#include <cstdint>
#include <cstring>



#if defined( _WIN32 ) && !defined( __MINGW32__ ) && !defined( __CYGWIN__ )
#line 1 "tao/pegtl/contrib/internal/endian_win.hpp"
       
#line 1 "tao/pegtl/contrib/internal/endian_win.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_WIN_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_WIN_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< std::size_t S >
   struct to_and_from_le
   {
      template< typename T >
      [[nodiscard]] static T convert( const T t ) noexcept
      {
         return t;
      }
   };

   template< std::size_t S >
   struct to_and_from_be;

   template<>
   struct to_and_from_be< 1 >
   {
      [[nodiscard]] static std::int8_t convert( const std::int8_t n ) noexcept
      {
         return n;
      }

      [[nodiscard]] static std::uint8_t convert( const std::uint8_t n ) noexcept
      {
         return n;
      }
   };

   template<>
   struct to_and_from_be< 2 >
   {
      [[nodiscard]] static std::int16_t convert( const std::int16_t n ) noexcept
      {
         return std::int16_t( _byteswap_ushort( std::uint16_t( n ) ) );
      }

      [[nodiscard]] static std::uint16_t convert( const std::uint16_t n ) noexcept
      {
         return _byteswap_ushort( n );
      }
   };

   template<>
   struct to_and_from_be< 4 >
   {
      [[nodiscard]] static float convert( float n ) noexcept
      {
         std::uint32_t u;
         std::memcpy( &u, &n, 4 );
         u = convert( u );
         std::memcpy( &n, &u, 4 );
         return n;
      }

      [[nodiscard]] static std::int32_t convert( const std::int32_t n ) noexcept
      {
         return std::int32_t( _byteswap_ulong( std::uint32_t( n ) ) );
      }

      [[nodiscard]] static std::uint32_t convert( const std::uint32_t n ) noexcept
      {
         return _byteswap_ulong( n );
      }
   };

   template<>
   struct to_and_from_be< 8 >
   {
      [[nodiscard]] static double convert( double n ) noexcept
      {
         std::uint64_t u;
         std::memcpy( &u, &n, 8 );
         u = convert( u );
         std::memcpy( &n, &u, 8 );
         return n;
      }

      [[nodiscard]] static std::int64_t convert( const std::int64_t n ) noexcept
      {
         return std::int64_t( _byteswap_uint64( std::uint64_t( n ) ) );
      }

      [[nodiscard]] static std::uint64_t convert( const std::uint64_t n ) noexcept
      {
         return _byteswap_uint64( n );
      }
   };

#define TAO_PEGTL_NATIVE_ORDER le
#define TAO_PEGTL_NATIVE_UTF16 utf16_le
#define TAO_PEGTL_NATIVE_UTF32 utf32_le

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/contrib/internal/endian.hpp"
#else
#line 1 "tao/pegtl/contrib/internal/endian_gcc.hpp"
       
#line 1 "tao/pegtl/contrib/internal/endian_gcc.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_GCC_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_ENDIAN_GCC_HPP

#include <cstdint>
#include <cstring>

namespace TAO_PEGTL_NAMESPACE::internal
{
#if !defined( __BYTE_ORDER__ )
#error No byte order defined!
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

   template< std::size_t S >
   struct to_and_from_be
   {
      template< typename T >
      [[nodiscard]] static T convert( const T n ) noexcept
      {
         return n;
      }
   };

   template< std::size_t S >
   struct to_and_from_le;

   template<>
   struct to_and_from_le< 1 >
   {
      [[nodiscard]] static std::uint8_t convert( const std::uint8_t n ) noexcept
      {
         return n;
      }

      [[nodiscard]] static std::int8_t convert( const std::int8_t n ) noexcept
      {
         return n;
      }
   };

   template<>
   struct to_and_from_le< 2 >
   {
      [[nodiscard]] static std::int16_t convert( const std::int16_t n ) noexcept
      {
         return static_cast< std::int16_t >( __builtin_bswap16( static_cast< std::uint16_t >( n ) ) );
      }

      [[nodiscard]] static std::uint16_t convert( const std::uint16_t n ) noexcept
      {
         return __builtin_bswap16( n );
      }
   };

   template<>
   struct to_and_from_le< 4 >
   {
      [[nodiscard]] static float convert( float n ) noexcept
      {
         std::uint32_t u;
         std::memcpy( &u, &n, 4 );
         u = convert( u );
         std::memcpy( &n, &u, 4 );
         return n;
      }

      [[nodiscard]] static std::int32_t convert( const std::int32_t n ) noexcept
      {
         return static_cast< std::int32_t >( __builtin_bswap32( static_cast< std::uint32_t >( n ) ) );
      }

      [[nodiscard]] static std::uint32_t convert( const std::uint32_t n ) noexcept
      {
         return __builtin_bswap32( n );
      }
   };

   template<>
   struct to_and_from_le< 8 >
   {
      [[nodiscard]] static double convert( double n ) noexcept
      {
         std::uint64_t u;
         std::memcpy( &u, &n, 8 );
         u = convert( u );
         std::memcpy( &n, &u, 8 );
         return n;
      }

      [[nodiscard]] static std::int64_t convert( const std::int64_t n ) noexcept
      {
         return static_cast< std::int64_t >( __builtin_bswap64( static_cast< std::uint64_t >( n ) ) );
      }

      [[nodiscard]] static std::uint64_t convert( const std::uint64_t n ) noexcept
      {
         return __builtin_bswap64( n );
      }
   };

#define TAO_PEGTL_NATIVE_ORDER be
#define TAO_PEGTL_NATIVE_UTF16 utf16_be
#define TAO_PEGTL_NATIVE_UTF32 utf32_be

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

   template< std::size_t S >
   struct to_and_from_le
   {
      template< typename T >
      [[nodiscard]] static T convert( const T n ) noexcept
      {
         return n;
      }
   };

   template< std::size_t S >
   struct to_and_from_be;

   template<>
   struct to_and_from_be< 1 >
   {
      [[nodiscard]] static std::int8_t convert( const std::int8_t n ) noexcept
      {
         return n;
      }

      [[nodiscard]] static std::uint8_t convert( const std::uint8_t n ) noexcept
      {
         return n;
      }
   };

   template<>
   struct to_and_from_be< 2 >
   {
      [[nodiscard]] static std::int16_t convert( const std::int16_t n ) noexcept
      {
         return static_cast< std::int16_t >( __builtin_bswap16( static_cast< std::uint16_t >( n ) ) );
      }

      [[nodiscard]] static std::uint16_t convert( const std::uint16_t n ) noexcept
      {
         return __builtin_bswap16( n );
      }
   };

   template<>
   struct to_and_from_be< 4 >
   {
      [[nodiscard]] static float convert( float n ) noexcept
      {
         std::uint32_t u;
         std::memcpy( &u, &n, 4 );
         u = convert( u );
         std::memcpy( &n, &u, 4 );
         return n;
      }

      [[nodiscard]] static std::int32_t convert( const std::int32_t n ) noexcept
      {
         return static_cast< std::int32_t >( __builtin_bswap32( static_cast< std::uint32_t >( n ) ) );
      }

      [[nodiscard]] static std::uint32_t convert( const std::uint32_t n ) noexcept
      {
         return __builtin_bswap32( n );
      }
   };

   template<>
   struct to_and_from_be< 8 >
   {
      [[nodiscard]] static double convert( double n ) noexcept
      {
         std::uint64_t u;
         std::memcpy( &u, &n, 8 );
         u = convert( u );
         std::memcpy( &n, &u, 8 );
         return n;
      }

      [[nodiscard]] static std::int64_t convert( const std::int64_t n ) noexcept
      {
         return static_cast< std::int64_t >( __builtin_bswap64( static_cast< std::uint64_t >( n ) ) );
      }

      [[nodiscard]] static std::uint64_t convert( const std::uint64_t n ) noexcept
      {
         return __builtin_bswap64( n );
      }
   };

#define TAO_PEGTL_NATIVE_ORDER le
#define TAO_PEGTL_NATIVE_UTF16 utf16_le
#define TAO_PEGTL_NATIVE_UTF32 utf32_le

#else
#error Unknown host byte order!
#endif

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 16 "tao/pegtl/contrib/internal/endian.hpp"
#endif

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename N >
   [[nodiscard]] N h_to_be( const N n ) noexcept
   {
      return N( to_and_from_be< sizeof( N ) >::convert( n ) );
   }

   template< typename N >
   [[nodiscard]] N be_to_h( const N n ) noexcept
   {
      return h_to_be( n );
   }

   template< typename N >
   [[nodiscard]] N be_to_h( const void* p ) noexcept
   {
      N n;
      std::memcpy( &n, p, sizeof( n ) );
      return internal::be_to_h( n );
   }

   template< typename N >
   [[nodiscard]] N h_to_le( const N n ) noexcept
   {
      return N( to_and_from_le< sizeof( N ) >::convert( n ) );
   }

   template< typename N >
   [[nodiscard]] N le_to_h( const N n ) noexcept
   {
      return h_to_le( n );
   }

   template< typename N >
   [[nodiscard]] N le_to_h( const void* p ) noexcept
   {
      N n;
      std::memcpy( &n, p, sizeof( n ) );
      return internal::le_to_h( n );
   }

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/contrib/internal/read_uint.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   struct read_uint16_be
   {
      using type = std::uint16_t;

      [[nodiscard]] static std::uint16_t read( const void* d ) noexcept
      {
         return be_to_h< std::uint16_t >( d );
      }
   };

   struct read_uint16_le
   {
      using type = std::uint16_t;

      [[nodiscard]] static std::uint16_t read( const void* d ) noexcept
      {
         return le_to_h< std::uint16_t >( d );
      }
   };

   struct read_uint32_be
   {
      using type = std::uint32_t;

      [[nodiscard]] static std::uint32_t read( const void* d ) noexcept
      {
         return be_to_h< std::uint32_t >( d );
      }
   };

   struct read_uint32_le
   {
      using type = std::uint32_t;

      [[nodiscard]] static std::uint32_t read( const void* d ) noexcept
      {
         return le_to_h< std::uint32_t >( d );
      }
   };

   struct read_uint64_be
   {
      using type = std::uint64_t;

      [[nodiscard]] static std::uint64_t read( const void* d ) noexcept
      {
         return be_to_h< std::uint64_t >( d );
      }
   };

   struct read_uint64_le
   {
      using type = std::uint64_t;

      [[nodiscard]] static std::uint64_t read( const void* d ) noexcept
      {
         return le_to_h< std::uint64_t >( d );
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 14 "tao/pegtl/contrib/internal/peek_mask_uint.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename R, typename R::type M >
   struct peek_mask_uint_impl
   {
      using data_t = typename R::type;
      using pair_t = input_pair< data_t >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.size( sizeof( data_t ) ) ) )
      {
         if( in.size( sizeof( data_t ) ) < sizeof( data_t ) ) {
            return { 0, 0 };
         }
         const data_t data = R::read( in.current() ) & M;
         return { data, sizeof( data_t ) };
      }
   };

   template< std::uint16_t M >
   using peek_mask_uint16_be = peek_mask_uint_impl< read_uint16_be, M >;

   template< std::uint16_t M >
   using peek_mask_uint16_le = peek_mask_uint_impl< read_uint16_le, M >;

   template< std::uint32_t M >
   using peek_mask_uint32_be = peek_mask_uint_impl< read_uint32_be, M >;

   template< std::uint32_t M >
   using peek_mask_uint32_le = peek_mask_uint_impl< read_uint32_le, M >;

   template< std::uint64_t M >
   using peek_mask_uint64_be = peek_mask_uint_impl< read_uint64_be, M >;

   template< std::uint64_t M >
   using peek_mask_uint64_le = peek_mask_uint_impl< read_uint64_le, M >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/contrib/uint16.hpp"
#line 1 "tao/pegtl/contrib/internal/peek_uint.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_uint.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UINT_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UINT_HPP

#include <cstddef>
#include <cstdint>






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename R >
   struct peek_uint_impl
   {
      using data_t = typename R::type;
      using pair_t = input_pair< data_t >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.size( sizeof( data_t ) ) ) )
      {
         if( in.size( sizeof( data_t ) ) < sizeof( data_t ) ) {
            return { 0, 0 };
         }
         const data_t data = R::read( in.current() );
         return { data, sizeof( data_t ) };
      }
   };

   using peek_uint16_be = peek_uint_impl< read_uint16_be >;
   using peek_uint16_le = peek_uint_impl< read_uint16_le >;

   using peek_uint32_be = peek_uint_impl< read_uint32_be >;
   using peek_uint32_le = peek_uint_impl< read_uint32_le >;

   using peek_uint64_be = peek_uint_impl< read_uint64_be >;
   using peek_uint64_le = peek_uint_impl< read_uint64_le >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/contrib/uint16.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace uint16_be
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint16_be > {};

      template< std::uint16_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint16_be, Cs... > {};
      template< std::uint16_t Lo, std::uint16_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint16_be, Lo, Hi > {};
      template< std::uint16_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint16_be, Cs... > {};
      template< std::uint16_t Lo, std::uint16_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint16_be, Lo, Hi > {};
      template< std::uint16_t... Cs > struct ranges : internal::ranges< internal::peek_uint16_be, Cs... > {};
      template< std::uint16_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint16_be, Cs >... > {};

      template< std::uint16_t M, std::uint16_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint16_be< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t Lo, std::uint16_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint16_be< M >, Lo, Hi > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint16_be< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t Lo, std::uint16_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint16_be< M >, Lo, Hi > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint16_be< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint16_be< M >, Cs >... > {};
      // clang-format on

   } // namespace uint16_be

   namespace uint16_le
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint16_le > {};

      template< std::uint16_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint16_le, Cs... > {};
      template< std::uint16_t Lo, std::uint16_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint16_le, Lo, Hi > {};
      template< std::uint16_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint16_le, Cs... > {};
      template< std::uint16_t Lo, std::uint16_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint16_le, Lo, Hi > {};
      template< std::uint16_t... Cs > struct ranges : internal::ranges< internal::peek_uint16_le, Cs... > {};
      template< std::uint16_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint16_le, Cs >... > {};

      template< std::uint16_t M, std::uint16_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint16_le< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t Lo, std::uint16_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint16_le< M >, Lo, Hi > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint16_le< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t Lo, std::uint16_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint16_le< M >, Lo, Hi > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint16_le< M >, Cs... > {};
      template< std::uint16_t M, std::uint16_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint16_le< M >, Cs >... > {};
      // clang-format on

   } // namespace uint16_le

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 28 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/uint32.hpp"
       
#line 1 "tao/pegtl/contrib/uint32.hpp"



#ifndef TAO_PEGTL_CONTRIB_UINT32_HPP
#define TAO_PEGTL_CONTRIB_UINT32_HPP
#line 14 "tao/pegtl/contrib/uint32.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   namespace uint32_be
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint32_be > {};

      template< std::uint32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint32_be, Cs... > {};
      template< std::uint32_t Lo, std::uint32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint32_be, Lo, Hi > {};
      template< std::uint32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint32_be, Cs... > {};
      template< std::uint32_t Lo, std::uint32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint32_be, Lo, Hi > {};
      template< std::uint32_t... Cs > struct ranges : internal::ranges< internal::peek_uint32_be, Cs... > {};
      template< std::uint32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint32_be, Cs >... > {};

      template< std::uint32_t M, std::uint32_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint32_be< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t Lo, std::uint32_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint32_be< M >, Lo, Hi > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint32_be< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t Lo, std::uint32_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint32_be< M >, Lo, Hi > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint32_be< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint32_be< M >, Cs >... > {};
      // clang-format on

   } // namespace uint32_be

   namespace uint32_le
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint32_le > {};

      template< std::uint32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint32_le, Cs... > {};
      template< std::uint32_t Lo, std::uint32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint32_le, Lo, Hi > {};
      template< std::uint32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint32_le, Cs... > {};
      template< std::uint32_t Lo, std::uint32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint32_le, Lo, Hi > {};
      template< std::uint32_t... Cs > struct ranges : internal::ranges< internal::peek_uint32_le, Cs... > {};
      template< std::uint32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint32_le, Cs >... > {};

      template< std::uint32_t M, std::uint32_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint32_le< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t Lo, std::uint32_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint32_le< M >, Lo, Hi > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint32_le< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t Lo, std::uint32_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint32_le< M >, Lo, Hi > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint32_le< M >, Cs... > {};
      template< std::uint32_t M, std::uint32_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint32_le< M >, Cs >... > {};
      // clang-format on

   } // namespace uint32_le

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 29 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/uint64.hpp"
       
#line 1 "tao/pegtl/contrib/uint64.hpp"



#ifndef TAO_PEGTL_CONTRIB_UINT64_HPP
#define TAO_PEGTL_CONTRIB_UINT64_HPP
#line 14 "tao/pegtl/contrib/uint64.hpp"
namespace TAO_PEGTL_NAMESPACE
{
   namespace uint64_be
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint64_be > {};

      template< std::uint64_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint64_be, Cs... > {};
      template< std::uint64_t Lo, std::uint64_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint64_be, Lo, Hi > {};
      template< std::uint64_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint64_be, Cs... > {};
      template< std::uint64_t Lo, std::uint64_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint64_be, Lo, Hi > {};
      template< std::uint64_t... Cs > struct ranges : internal::ranges< internal::peek_uint64_be, Cs... > {};
      template< std::uint64_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint64_be, Cs >... > {};


      template< std::uint64_t M, std::uint64_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint64_be< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t Lo, std::uint64_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint64_be< M >, Lo, Hi > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint64_be< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t Lo, std::uint64_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint64_be< M >, Lo, Hi > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint64_be< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint64_be< M >, Cs >... > {};
      // clang-format on

   } // namespace uint64_be

   namespace uint64_le
   {
      // clang-format off
      struct any : internal::any< internal::peek_uint64_le > {};

      template< std::uint64_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint64_le, Cs... > {};
      template< std::uint64_t Lo, std::uint64_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint64_le, Lo, Hi > {};
      template< std::uint64_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint64_le, Cs... > {};
      template< std::uint64_t Lo, std::uint64_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint64_le, Lo, Hi > {};
      template< std::uint64_t... Cs > struct ranges : internal::ranges< internal::peek_uint64_le, Cs... > {};
      template< std::uint64_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint64_le, Cs >... > {};

      template< std::uint64_t M, std::uint64_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint64_le< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t Lo, std::uint64_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint64_le< M >, Lo, Hi > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint64_le< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t Lo, std::uint64_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint64_le< M >, Lo, Hi > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint64_le< M >, Cs... > {};
      template< std::uint64_t M, std::uint64_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint64_le< M >, Cs >... > {};
      // clang-format on

   } // namespace uint64_le

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 30 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/uint8.hpp"
       
#line 1 "tao/pegtl/contrib/uint8.hpp"



#ifndef TAO_PEGTL_CONTRIB_UINT8_HPP
#define TAO_PEGTL_CONTRIB_UINT8_HPP





#line 1 "tao/pegtl/contrib/internal/peek_mask_uint8.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_mask_uint8.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_MASK_UINT8_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_MASK_UINT8_HPP

#include <cstddef>
#include <cstdint>




namespace TAO_PEGTL_NAMESPACE::internal
{
   template< std::uint8_t M >
   struct peek_mask_uint8
   {
      using data_t = std::uint8_t;
      using pair_t = input_pair< std::uint8_t >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( in.empty() ) {
            return { 0, 0 };
         }
         return { std::uint8_t( in.peek_uint8() & M ), 1 };
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/contrib/uint8.hpp"
#line 1 "tao/pegtl/contrib/internal/peek_uint8.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_uint8.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UINT8_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UINT8_HPP

#include <cstddef>
#include <cstdint>




namespace TAO_PEGTL_NAMESPACE::internal
{
   struct peek_uint8
   {
      using data_t = std::uint8_t;
      using pair_t = input_pair< std::uint8_t >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.empty() ) )
      {
         if( in.empty() ) {
            return { 0, 0 };
         }
         return { in.peek_uint8(), 1 };
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 13 "tao/pegtl/contrib/uint8.hpp"

namespace TAO_PEGTL_NAMESPACE::uint8
{
   // clang-format off
   struct any : internal::any< internal::peek_uint8 > {};

   template< std::uint8_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_uint8, Cs... > {};
   template< std::uint8_t Lo, std::uint8_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_uint8, Lo, Hi > {};
   template< std::uint8_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_uint8, Cs... > {};
   template< std::uint8_t Lo, std::uint8_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_uint8, Lo, Hi > {};
   template< std::uint8_t... Cs > struct ranges : internal::ranges< internal::peek_uint8, Cs... > {};
   template< std::uint8_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_uint8, Cs >... > {};

   template< std::uint8_t M, std::uint8_t... Cs > struct mask_not_one : internal::one< internal::result_on_found::failure, internal::peek_mask_uint8< M >, Cs... > {};
   template< std::uint8_t M, std::uint8_t Lo, std::uint8_t Hi > struct mask_not_range : internal::range< internal::result_on_found::failure, internal::peek_mask_uint8< M >, Lo, Hi > {};
   template< std::uint8_t M, std::uint8_t... Cs > struct mask_one : internal::one< internal::result_on_found::success, internal::peek_mask_uint8< M >, Cs... > {};
   template< std::uint8_t M, std::uint8_t Lo, std::uint8_t Hi > struct mask_range : internal::range< internal::result_on_found::success, internal::peek_mask_uint8< M >, Lo, Hi > {};
   template< std::uint8_t M, std::uint8_t... Cs > struct mask_ranges : internal::ranges< internal::peek_mask_uint8< M >, Cs... > {};
   template< std::uint8_t M, std::uint8_t... Cs > struct mask_string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_mask_uint8< M >, Cs >... > {};
   // clang-format on

} // namespace TAO_PEGTL_NAMESPACE::uint8

#endif
#line 31 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/unescape.hpp"
       
#line 1 "tao/pegtl/contrib/unescape.hpp"



#ifndef TAO_PEGTL_CONTRIB_UNESCAPE_HPP
#define TAO_PEGTL_CONTRIB_UNESCAPE_HPP

#include <cassert>
#include <string>





namespace TAO_PEGTL_NAMESPACE::unescape
{
   // Utility functions for the unescape actions.

   [[nodiscard]] inline bool utf8_append_utf32( std::string& string, const unsigned utf32 )
   {
      if( utf32 <= 0x7f ) {
         string += char( utf32 & 0xff );
         return true;
      }
      if( utf32 <= 0x7ff ) {
         char tmp[] = { char( ( ( utf32 & 0x7c0 ) >> 6 ) | 0xc0 ),
                        char( ( ( utf32 & 0x03f ) ) | 0x80 ) };
         string.append( tmp, sizeof( tmp ) );
         return true;
      }
      if( utf32 <= 0xffff ) {
         if( utf32 >= 0xd800 && utf32 <= 0xdfff ) {
            // nope, this is a UTF-16 surrogate
            return false;
         }
         char tmp[] = { char( ( ( utf32 & 0xf000 ) >> 12 ) | 0xe0 ),
                        char( ( ( utf32 & 0x0fc0 ) >> 6 ) | 0x80 ),
                        char( ( ( utf32 & 0x003f ) ) | 0x80 ) };
         string.append( tmp, sizeof( tmp ) );
         return true;
      }
      if( utf32 <= 0x10ffff ) {
         char tmp[] = { char( ( ( utf32 & 0x1c0000 ) >> 18 ) | 0xf0 ),
                        char( ( ( utf32 & 0x03f000 ) >> 12 ) | 0x80 ),
                        char( ( ( utf32 & 0x000fc0 ) >> 6 ) | 0x80 ),
                        char( ( ( utf32 & 0x00003f ) ) | 0x80 ) };
         string.append( tmp, sizeof( tmp ) );
         return true;
      }
      return false;
   }

   // This function MUST only be called for characters matching TAO_PEGTL_NAMESPACE::ascii::xdigit!
   template< typename I >
   [[nodiscard]] I unhex_char( const char c )
   {
      switch( c ) {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            return I( c - '0' );
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
            return I( c - 'a' + 10 );
         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
            return I( c - 'A' + 10 );
         default: // LCOV_EXCL_LINE
            throw std::runtime_error( "invalid character in unhex" ); // LCOV_EXCL_LINE
      }
   }

   template< typename I >
   [[nodiscard]] I unhex_string( const char* begin, const char* end )
   {
      I r = 0;
      while( begin != end ) {
         r <<= 4;
         r += unhex_char< I >( *begin++ );
      }
      return r;
   }

   // Actions for common unescape situations.

   struct append_all
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& s )
      {
         s.append( in.begin(), in.size() );
      }
   };

   // This action MUST be called for a character matching T which MUST be TAO_PEGTL_NAMESPACE::one< ... >.
   template< typename T, char... Rs >
   struct unescape_c
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& s )
      {
         assert( in.size() == 1 );
         s += apply_one( in, static_cast< const T* >( nullptr ) );
      }

      template< typename ActionInput, char... Qs >
      [[nodiscard]] static char apply_one( const ActionInput& in, const one< Qs... >* /*unused*/ )
      {
         static_assert( sizeof...( Qs ) == sizeof...( Rs ), "size mismatch between escaped characters and their mappings" );
         return apply_two( in, { Qs... }, { Rs... } );
      }

      template< typename ActionInput >
      [[nodiscard]] static char apply_two( const ActionInput& in, const std::initializer_list< char >& q, const std::initializer_list< char >& r )
      {
         const char c = *in.begin();
         for( std::size_t i = 0; i < q.size(); ++i ) {
            if( *( q.begin() + i ) == c ) {
               return *( r.begin() + i );
            }
         }
         throw parse_error( "invalid character in unescape", in ); // LCOV_EXCL_LINE
      }
   };

   // See src/example/pegtl/unescape.cpp for why the following two actions
   // skip the first input character. They also MUST be called
   // with non-empty matched inputs!

   struct unescape_u
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& s )
      {
         assert( !in.empty() ); // First character MUST be present, usually 'u' or 'U'.
         if( !utf8_append_utf32( s, unhex_string< unsigned >( in.begin() + 1, in.end() ) ) ) {
            throw parse_error( "invalid escaped unicode code point", in );
         }
      }
   };

   struct unescape_x
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& s )
      {
         assert( !in.empty() ); // First character MUST be present, usually 'x'.
         s += unhex_string< char >( in.begin() + 1, in.end() );
      }
   };

   // The unescape_j action is similar to unescape_u, however unlike
   // unescape_u it
   // (a) assumes exactly 4 hexdigits per escape sequence,
   // (b) accepts multiple consecutive escaped 16-bit values.
   // When applied to more than one escape sequence, unescape_j
   // translates UTF-16 surrogate pairs in the input into a single
   // UTF-8 sequence in s, as required for JSON by RFC 8259.

   struct unescape_j
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& s )
      {
         assert( ( ( in.size() + 1 ) % 6 ) == 0 ); // Expects multiple "\\u1234", starting with the first "u".
         for( const char* b = in.begin() + 1; b < in.end(); b += 6 ) {
            const auto c = unhex_string< unsigned >( b, b + 4 );
            if( ( 0xd800 <= c ) && ( c <= 0xdbff ) && ( b + 6 < in.end() ) ) {
               const auto d = unhex_string< unsigned >( b + 6, b + 10 );
               if( ( 0xdc00 <= d ) && ( d <= 0xdfff ) ) {
                  b += 6;
                  (void)utf8_append_utf32( s, ( ( ( c & 0x03ff ) << 10 ) | ( d & 0x03ff ) ) + 0x10000 );
                  continue;
               }
            }
            if( !utf8_append_utf32( s, c ) ) {
               throw parse_error( "invalid escaped unicode code point", in );
            }
         }
      }
   };

} // namespace TAO_PEGTL_NAMESPACE::unescape

#endif
#line 32 "amalgamated.hpp"

#line 1 "tao/pegtl/contrib/utf16.hpp"
       
#line 1 "tao/pegtl/contrib/utf16.hpp"



#ifndef TAO_PEGTL_CONTRIB_UTF16_HPP
#define TAO_PEGTL_CONTRIB_UTF16_HPP





#line 1 "tao/pegtl/contrib/internal/peek_utf16.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_utf16.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UTF16_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UTF16_HPP

#include <type_traits>






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename R >
   struct peek_utf16_impl
   {
      using data_t = char32_t;
      using pair_t = input_pair< char32_t >;

      using short_t = std::make_unsigned< char16_t >::type;

      static_assert( sizeof( short_t ) == 2 );
      static_assert( sizeof( char16_t ) == 2 );

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.size( 4 ) ) )
      {
         if( in.size( 2 ) < 2 ) {
            return { 0, 0 };
         }
         const char32_t t = R::read( in.current() );
         if( ( t < 0xd800 ) || ( t > 0xdfff ) ) {
            return { t, 2 };
         }
         if( ( t >= 0xdc00 ) || ( in.size( 4 ) < 4 ) ) {
            return { 0, 0 };
         }
         const char32_t u = R::read( in.current() + 2 );
         if( ( u >= 0xdc00 ) && ( u <= 0xdfff ) ) {
            const auto cp = ( ( ( t & 0x03ff ) << 10 ) | ( u & 0x03ff ) ) + 0x10000;
            return { cp, 4 };
         }
         return { 0, 0 };
      }
   };

   using peek_utf16_be = peek_utf16_impl< read_uint16_be >;
   using peek_utf16_le = peek_utf16_impl< read_uint16_le >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/contrib/utf16.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace utf16_be
   {
      // clang-format off
      struct any : internal::any< internal::peek_utf16_be > {};
      struct bom : internal::one< internal::result_on_found::success, internal::peek_utf16_be, 0xfeff > {};
      template< char32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_utf16_be, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_utf16_be, Lo, Hi > {};
      template< char32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_utf16_be, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_utf16_be, Lo, Hi > {};
      template< char32_t... Cs > struct ranges : internal::ranges< internal::peek_utf16_be, Cs... > {};
      template< char32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_utf16_be, Cs >... > {};
      // clang-format on

   } // namespace utf16_be

   namespace utf16_le
   {
      // clang-format off
      struct any : internal::any< internal::peek_utf16_le > {};
      struct bom : internal::one< internal::result_on_found::success, internal::peek_utf16_le, 0xfeff > {};
      template< char32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_utf16_le, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_utf16_le, Lo, Hi > {};
      template< char32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_utf16_le, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_utf16_le, Lo, Hi > {};
      template< char32_t... Cs > struct ranges : internal::ranges< internal::peek_utf16_le, Cs... > {};
      template< char32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_utf16_le, Cs >... > {};
      // clang-format on

   } // namespace utf16_le

   namespace utf16 = TAO_PEGTL_NATIVE_UTF16;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 34 "amalgamated.hpp"
#line 1 "tao/pegtl/contrib/utf32.hpp"
       
#line 1 "tao/pegtl/contrib/utf32.hpp"



#ifndef TAO_PEGTL_CONTRIB_UTF32_HPP
#define TAO_PEGTL_CONTRIB_UTF32_HPP





#line 1 "tao/pegtl/contrib/internal/peek_utf32.hpp"
       
#line 1 "tao/pegtl/contrib/internal/peek_utf32.hpp"



#ifndef TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UTF32_HPP
#define TAO_PEGTL_CONTRIB_INTERNAL_PEEK_UTF32_HPP

#include <cstddef>






namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename R >
   struct peek_utf32_impl
   {
      using data_t = char32_t;
      using pair_t = input_pair< char32_t >;

      static_assert( sizeof( char32_t ) == 4 );

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in ) noexcept( noexcept( in.size( 4 ) ) )
      {
         if( in.size( 4 ) < 4 ) {
            return { 0, 0 };
         }
         const char32_t t = R::read( in.current() );
         if( ( t <= 0x10ffff ) && !( t >= 0xd800 && t <= 0xdfff ) ) {
            return { t, 4 };
         }
         return { 0, 0 };
      }
   };

   using peek_utf32_be = peek_utf32_impl< read_uint32_be >;
   using peek_utf32_le = peek_utf32_impl< read_uint32_le >;

} // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#line 12 "tao/pegtl/contrib/utf32.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace utf32_be
   {
      // clang-format off
      struct any : internal::any< internal::peek_utf32_be > {};
      struct bom : internal::one< internal::result_on_found::success, internal::peek_utf32_be, 0xfeff > {};
      template< char32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_utf32_be, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_utf32_be, Lo, Hi > {};
      template< char32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_utf32_be, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_utf32_be, Lo, Hi > {};
      template< char32_t... Cs > struct ranges : internal::ranges< internal::peek_utf32_be, Cs... > {};
      template< char32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_utf32_be, Cs >... > {};
      // clang-format on

   } // namespace utf32_be

   namespace utf32_le
   {
      // clang-format off
      struct any : internal::any< internal::peek_utf32_le > {};
      struct bom : internal::one< internal::result_on_found::success, internal::peek_utf32_le, 0xfeff > {};
      template< char32_t... Cs > struct not_one : internal::one< internal::result_on_found::failure, internal::peek_utf32_le, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct not_range : internal::range< internal::result_on_found::failure, internal::peek_utf32_le, Lo, Hi > {};
      template< char32_t... Cs > struct one : internal::one< internal::result_on_found::success, internal::peek_utf32_le, Cs... > {};
      template< char32_t Lo, char32_t Hi > struct range : internal::range< internal::result_on_found::success, internal::peek_utf32_le, Lo, Hi > {};
      template< char32_t... Cs > struct ranges : internal::ranges< internal::peek_utf32_le, Cs... > {};
      template< char32_t... Cs > struct string : internal::seq< internal::one< internal::result_on_found::success, internal::peek_utf32_le, Cs >... > {};
      // clang-format on

   } // namespace utf32_le

   namespace utf32 = TAO_PEGTL_NATIVE_UTF32;

} // namespace TAO_PEGTL_NAMESPACE

#endif
#line 34 "amalgamated.hpp"
