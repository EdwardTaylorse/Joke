#pragma once 
#include <vector>
#include <fc/optional.hpp>
#include <fc/string.hpp>
#include <memory>
#include <string.h> // memset
#include <unordered_set>
#include <set>

namespace fc
{
   /**
    * @defgroup serializable Serializable _types
    * @brief Clas_ses that may be converted to/from an variant
    *
    * To make a class 'serializable' the following methods must be available 
    * for your Serializable_type
    *
    *  @code 
    *     void   to_variant( const Serializable_type& e, variant& v );
    *     void   from_variant( const variant& e, Serializable_type& ll );
    *  @endcode
    */

   class variant;
   class variant_object;
   class mutable_variant_object;
   class time_point;

   void to_variant( const uint16_t& var,  variant& vo );
   void from_variant( const variant& var,  uint16_t& vo );
   void to_variant( const uint32_t& var,  variant& vo );
   void from_variant( const variant& var,  uint32_t& vo );
   void to_variant( const uint8_t& var,  variant& vo );
   void from_variant( const variant& var,  uint8_t& vo );

   void to_variant( const variant_object& var,  variant& vo );
   void from_variant( const variant& var,  variant_object& vo );
   void to_variant( const mutable_variant_object& var,  variant& vo );
   void from_variant( const variant& var,  mutable_variant_object& vo );
   void to_variant( const std::vector<char>& var,  variant& vo );
   void from_variant( const variant& var,  std::vector<char>& vo );

   template<typename T>
   void to_variant( const std::unordered_set<T>& var,  variant& vo );
   template<typename T>
   void from_variant( const variant& var,  std::unordered_set<T>& vo );
   template<typename T>
   void to_variant( const std::set<T>& var,  variant& vo );
   template<typename T>
   void from_variant( const variant& var,  std::set<T>& vo );

   void to_variant( const time_point& var,  variant& vo );
   void from_variant( const variant& var,  time_point& vo );
   #ifdef __APPLE__
   void to_variant( size_t s, variant& v );
   #endif
   void to_variant( const std::string& s, variant& v );

   template<typename T>
   void to_variant( const std::shared_ptr<T>& var,  variant& vo );

   template<typename T>
   void from_variant( const variant& var,  std::shared_ptr<T>& vo );

   typedef std::vector<variant>   variants;

   /**
    * @brief stores null, int64, uint64, double, bool, string, std::vector<variant>,
    *        and variant_object's.  
    *
    * variant's allocate everything but strings, arrays, and objects on the
    * stack and are 'move aware' for values allcoated on the heap.  
    *
    * Memory usage on 64 bit systems is 16 bytes and 12 bytes on 32 bit systems.
    */
   class variant
   {
      public:
        enum type_id
        {
           null_type   = 0,     
           int64_type  = 1, 
           uint64_type = 2, 
           double_type = 3,
           bool_type   = 4,
           string_type = 5,
           array_type  = 6,
           object_type = 7
        };

        /// Constructs a null_type variant
        variant();
        /// Constructs a null_type variant
        variant( nullptr_t );

        /// @param str - UTF8 string
        variant( const char* str );
        variant( char* str );
	variant( wchar_t* str );
	variant( const wchar_t* str );
        variant( int val );
        variant( float val );
        variant( int64_t val );
        variant( uint64_t val );
        variant( double val );
        variant( bool val );
        variant( fc::string val );
        variant( variant_object );
        variant( mutable_variant_object );
        variant( variants );
        variant( const variant& );
        variant( variant&& );
       ~variant();

        /**
         *  Read-only access to the content of the variant.
         */
        class visitor 
        {
           public:
              virtual ~visitor(){}
              /// handles null_type variants
              virtual void handle()const                         = 0;
              virtual void handle( const int64_t& v )const       = 0;
              virtual void handle( const uint64_t& v )const      = 0;
              virtual void handle( const double& v )const        = 0;
              virtual void handle( const bool& v )const          = 0;
              virtual void handle( const string& v )const        = 0;
              virtual void handle( const variant_object& v)const = 0;
              virtual void handle( const variants& v)const       = 0;
        };

        void  visit( const visitor& v )const;

        type_id                     get_type()const;

        bool                        is_null()const;
        bool                        is_string()const;
        bool                        is_bool()const;
        bool                        is_int64()const;
        bool                        is_uint64()const;
        bool                        is_double()const;
        bool                        is_object()const;
        bool                        is_array()const;
        /**
         *   int64, uint64, double,bool
         */
        bool                        is_numeric()const;
                                    
        int64_t                     as_int64()const;
        uint64_t                    as_uint64()const;
        bool                        as_bool()const;
        double                      as_double()const;

        /** Convert's double, ints, bools, etc to a string
         * @throw if get_type() == array_type | get_type() == object_type 
         */
        string                      as_string()const;

        /// @pre  get_type() == string_type
        const string&               get_string()const;
                                    
        /// @throw if get_type() != array_type | null_type
        variants&                   get_array();

        /// @throw if get_type() != array_type 
        const variants&             get_array()const;

        /// @throw if get_type() != object_type | null_type
        variant_object&             get_object();

        /// @throw if get_type() != object_type 
        const variant_object&       get_object()const;

        /// @pre is_object()
        const variant&              operator[]( const char* )const;
        /// @pre is_array()
        const variant&              operator[]( size_t pos )const;
        /// @pre is_array()
        size_t                      size()const;

        /**
         *  _types that use non-intrusive variant conversion can implement the
         *  following method to implement conversion from variant to T.
         *
         *  <code>
         *  void from_variant( const Variant& var, T& val )
         *  </code>
         *
         *  The above form is not always convienant, so the this templated 
         *  method is used to enable conversion from Variants to other
         *  types.
         */
        template<typename T>
        T as()const
        {
           T tmp;
           from_variant( *this, tmp );
           return tmp;
        }

        variant& operator=( variant&& v );
        variant& operator=( const variant& v );

        template<typename T>
        variant& operator=( T&& v )
        {
           *this = variant( fc::forward<T>(v) );
           return *this;
        }

        template<typename T>
        variant( const optional<T>& v )
        {
           if( v ) *this = variant(*v);
        }

        template<typename T>
        explicit variant( const T& val );


      private:
        double  _data;                ///< Alligned according to double requirements
        char    _type[sizeof(void*)]; ///< pad to void* size
   };
   typedef optional<variant> ovariant;
  
   /** @ingroup Serializable */
   void from_variant( const variant& var,  string& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  variants& vo );
   void from_variant( const variant& var,  variant& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  int64_t& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  uint64_t& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  bool& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  double& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  float& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  int32_t& vo );
   /** @ingroup Serializable */
   void from_variant( const variant& var,  uint32_t& vo );
   /** @ingroup Serializable */
   template<typename T>
   void from_variant( const variant& var,  optional<T>& vo )
   {
      if( var.is_null() ) vo = optional<T>();
      else
      {
          vo = T();
          from_variant( var, *vo );
      }
   }
   template<typename T>
   void to_variant( const std::unordered_set<T>& var,  variant& vo )
   {
       std::vector<variant> vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = variant(*itr);
       vo = vars;
   }
   template<typename T>
   void from_variant( const variant& var,  std::unordered_set<T>& vo )
   {
      const variants& vars = var.get_array();
      vo.clear();
      vo.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as<T>() );
   }
   template<typename T>
   void to_variant( const std::set<T>& var,  variant& vo )
   {
       std::vector<variant> vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = variant(*itr);
       vo = vars;
   }
   template<typename T>
   void from_variant( const variant& var,  std::set<T>& vo )
   {
      const variants& vars = var.get_array();
      vo.clear();
      vo.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as<T>() );
   }

   /** @ingroup Serializable */
   template<typename T>
   void from_variant( const variant& var, std::vector<T>& tmp )
   {
      const variants& vars = var.get_array();
      tmp.clear();
      tmp.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         tmp.push_back( itr->as<T>() );
   }

   /** @ingroup Serializable */
   template<typename T>
   void to_variant( const std::vector<T>& t, variant& v )
   {
      std::vector<variant> vars(t.size());
       for( size_t i = 0; i < t.size(); ++i )
          vars[i] = variant(t[i]);
       v = vars;
   }


   template<typename T>
   variant::variant( const T& val )
   {
      memset( this, 0, sizeof(*this) );
      to_variant( val, *this );
   }
   #ifdef __APPLE__
   inline void to_variant( size_t s, variant& v ) { v = variant(uint64_t(s)); }
   #endif
   template<typename T>
   void to_variant( const std::shared_ptr<T>& var,  variant& vo )
   {
      if( var ) to_variant( *var, vo );
      else vo = nullptr;
   }

   template<typename T>
   void from_variant( const variant& var,  std::shared_ptr<T>& vo )
   {
      if( var.is_null() ) vo = nullptr;
      else if( vo ) from_variant( var, *vo );
      else {
          vo = std::make_shared<T>();
          from_variant( var, *vo );
      }
   }

} // namespace fc
