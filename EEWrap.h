/***
    EEWrap library.
    Copyright: Christopher Andrews.
    Licence: MIT
    Website: http://arduino.land/ (specific project page soon)
    Source:
    
    This library is a helper for AVR based EEPROM access.
    It allows using variables stored in the EEPROM as if they were in RAM,
    and essentially makes accessing EEPROM structures simple.
    
    Visit the source or website for usage instructions.
***/


#include <EEPROM.h>

#define STATIC_ACCESS   D& self() { return *static_cast< D* >( this ); } \
                        const D& self() const { return *static_cast< const D* >( this ); }

#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
    #ifndef ISCPP11
        #define ISCPP11
    #endif
#endif

#ifndef HEADER_EEWRAP
    #define HEADER_EEWRAP

template< typename T > struct EEWrap; //Forward declaration to allow EEMode use.

namespace ee{

    typedef uint8_t* uptr;

    /** Type traits for compile time optimization. **/
    namespace tools{

        /***
            Select different types based on a condition, an 'if statement' for types.
            The type Select<V,T,F>::Result is type T when V is true, otherwise it is set to type F.
        ***/
        template< bool V, typename T, typename F > struct select{ typedef T type; };
        template< typename T, typename F > struct select< false, T, F >{ typedef F type; };

        /** A basic comparison of two types **/
        template < typename T, typename U > struct is_same{ enum { value = false }; };
        template < typename T > struct is_same< T, T >    { enum { value = true }; };

        /***
            Determine if T is an integer type.
            is_integer<T>::value is true when T is an integer type (bool is not included even though it is an integral type), 
            anything else results in false.
        ***/
        template< typename T >
            struct is_integer{
                enum{
                    V8    = is_same< T, char >::value || is_same< T, unsigned char >::value  || is_same< T, signed char >::value,
                    V16   = is_same< T, unsigned int >::value || is_same< T, signed int >::value,
                    V32   = is_same< T, unsigned long >::value || is_same< T, signed long >::value,
                    V64   = is_same< T, unsigned long long >::value || is_same< T, signed long long >::value,
                    value = V8 || V16 || V32 || V64
                };
        };
        template< typename T > struct is_float{ enum{ value = is_same< T, float >::value || is_same< T, double >::value || is_same< T, long double >::value }; };
        template< typename T > struct is_number{ enum{ value = is_float< T >::value || is_integer< T >::value }; };
        template< typename T > struct is_bool{ enum{ value = is_same< T, bool >::value }; };

        template< typename T > struct is_fundamental{
            enum{
                value = is_number<T>::value || is_bool<T>::value || is_same< T, wchar_t >::value || is_same< T, char16_t >::value
                #ifdef ISCPP11
                    || is_same< T, char32_t >::value
                #endif
            };
        };
        /***
            Multi byte read/write raw methods (EEWrap specific).
            Reads raw object data in the EEPROM based on endianess of the AVR.
            These methods do not function like eeprom_write_block or EEPROM_Anything.
        ***/

        inline void EEReadBlockElement(  char *out, uint8_t *addr, const unsigned int len ){
            for( unsigned int i = 0 ; i < len ; ++i ) *out-- = EEPROM[int(addr++)];
        }

        inline void EEWriteBlockElement(  const char *in, uint8_t *addr, const unsigned int len ){
            for( unsigned int i = 0 ; i < len ; i++ ){
                const char b = *in;
                EEPROM[int(addr++)].update(b);
                --in;
            }
        }

    } //namespace tools

    /** Add-in methods for primitive types. **/
    template< typename D, typename T > class PrimitiveWrap{
        public:
            D &operator +=( const T &in ){ return self().EEWrite( self().EERead() + in ); }
            D &operator -=( const T &in ){ return self().EEWrite( self().EERead() - in ); }
            D &operator *=( const T &in ){ return self().EEWrite( self().EERead() * in ); }
            D &operator /=( const T &in ){ return self().EEWrite( self().EERead() / in ); }

            D &operator |=( const T &in ){ return self().EEWrite( self().EERead() | in ); }
            D &operator &=( const T &in ){ return self().EEWrite( self().EERead() & in ); }
            D &operator %=( const T &in ){ return self().EEWrite( self().EERead() % in ); }
            D &operator ^=( const T &in ){ return self().EEWrite( self().EERead() ^ in ); }

            template< typename U > D &operator <<=( const U &in ){ return self().EEWrite( self().EERead() << in ); }
            template< typename U > D &operator >>=( const U &in ){ return self().EEWrite( self().EERead() >> in ); }

            /** Prefix increment/decrement **/
            D& operator++ (){ return self().EEWrite( self().EERead() + 1 ); }
            D& operator-- (){ return self().EEWrite( self().EERead() - 1 ); }

            /** Postfix increment/decrement **/
            T  operator++ (int){
                T ret = self().EERead();
                self().EEWrite( ret + 1 );
                return ret;
            }

            T  operator-- (int){
                T ret = self().EERead();
                self().EEWrite( ret - 1 );
                return ret;
            }
        protected: STATIC_ACCESS
    };

    struct NoPrimitiveWrap{};

    /***
        Optimised selection of read/write routines.
    ***/

    template< typename D, typename T >
    class EEMultiByte{
        public:

        T EERead( void ){
            T ret;
            tools::EEReadBlockElement( (char*)&ret + sizeof(T) - 1, uptr(&self()), sizeof(T) );
            return(ret);
        }

        D &EEWrite( const T& v ){
            tools::EEWriteBlockElement( (const char*) &v + sizeof(T) - 1, uptr(&self()), sizeof(T) );
            return self();
        }
        protected: STATIC_ACCESS
    };

    template< typename D, typename T >
    class EESingleByte{
        public:

        T EERead( void ){ return EEPROM[int(&self())]; }

        D &EEWrite( const T& v ){
            EEPROM[int(&self())].update(v);
            return self();
        }
        protected: STATIC_ACCESS
    };

    /** EEMode is a helper to encapsulate functionality for primitive types **/
    template< typename T > struct EEMode{

        //Select the most appropriate method based on the size of T. No loops are needed for single byte types.
        typedef typename tools::select<
            sizeof(T) == 1,
            EESingleByte< EEWrap<T>, T>,
            EEMultiByte< EEWrap<T>, T >
        >::type Interface;

        //If T is a primitive type, the standard operators are exposed (class PrimitiveWrap).
        typedef typename tools::select<
            tools::is_fundamental<T>::value,
            PrimitiveWrap< EEWrap<T>, T >,
            NoPrimitiveWrap
        >::type Extension;
    };

} //namespace ee

    /***
        EEWrap class.
    ***/

    template< typename T >
        struct EEWrap :
            ee::EEMode<T>::Interface,
            ee::EEMode<T>::Extension{

        using ee::EEMode<T>::Interface::EERead;
        using ee::EEMode<T>::Interface::EEWrite;

        operator const T() { return EERead(); }
        EEWrap &operator =( const T &in ){  return EEWrite( in ); }
        bool operator ==( const T &in ){ return EERead() == in; }

        T t;
    };

    typedef EEWrap< char > int8_e;                   //char
    typedef EEWrap< unsigned char > uint8_e;         //unsigned char
    typedef EEWrap< int > int16_e;                   //int
    typedef EEWrap< unsigned int > uint16_e;         //unsigned int
    typedef EEWrap< long > int32_e;                  //long
    typedef EEWrap< unsigned long > uint32_e;        //unsigned long
    typedef EEWrap< long long > int64_e;             //long long
    typedef EEWrap< unsigned long long > uint64_e;   //unsigned long long
    typedef EEWrap< bool > bool_e;                   //bool
    typedef EEWrap< float > float_e;                 //float
    typedef EEWrap< double > double_e;               //double
    typedef EEWrap< long double > long_double_e;     //long double

    #undef STATIC_ACCESS
#endif