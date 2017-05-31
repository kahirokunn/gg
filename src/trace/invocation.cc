/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <sstream>
#include <string>

#include "invocation.hh"
#include "traced_process.hh"

using namespace std;

/* Argument */

template<>
void Argument::set_value( const string value )
{
  value_.string_val.reset( value );
}

template<>
void Argument::set_value( const long value )
{
  value_.long_val.reset( value );
}

template<> string Argument::value() const { return value_.string_val.get(); }
template<> long   Argument::value() const { return value_.long_val.get(); }
template<> int    Argument::value() const { return ( int )value_.long_val.get(); }

Argument::Argument( ArgumentInfo info, const long raw_value )
  : info_( info ), raw_value_( raw_value ), value_()
{}

/* SystemCallInvocation */

SystemCallInvocation::SystemCallInvocation( const pid_t pid,
                                            const long syscall_no )
  : pid_( pid ), syscall_( syscall_no ), signature_(), arguments_(),
    return_value_()
{
  if ( syscall_signature( syscall_no ).complete() ) {
    const SystemCallSignature & sig = syscall_signature( syscall_no );
    signature_.reset( sig );
  }
}

void SystemCallInvocation::fetch_arguments()
{
  const SystemCallSignature & sig = syscall_signature( syscall_ );
  vector<ArgumentInfo> args = sig.arguments();

  arguments_.reset();

  for ( size_t i = 0; i < args.size(); i++ ) {
    const ArgumentInfo & arg_info = args.at( i );

    arguments_->emplace_back( arg_info, TracedProcess::get_syscall_arg<long>( pid_, i ) );

    Argument & last_arg = arguments_->back();

    if ( arg_info.is_readable_string() ) {
      last_arg.set_value( TracedProcess::get_syscall_arg<string>( pid_, i ) );
    }

    // let's set the raw value anyway
    last_arg.set_value( last_arg.raw_value() );
  }
}

std::string SystemCallInvocation::name()
{
  if ( signature_.initialized() ) {
    return signature_->name();
  }
  else {
    ostringstream out;
    out << "sc-" << syscall_;
    return out.str();
  }
}

std::string SystemCallInvocation::to_string() const
{
  ostringstream out;

  if ( signature().initialized() ) {
    out << signature()->name() << "(";

    size_t i = 0;
    if ( arguments_.initialized() ) {
      for ( auto & arg : *arguments_ ) {
        i++;

        if ( arg.info().is_readable_string() ) {
          out << '"' << arg.value<string>() << '"';
        }
        else {
          out << arg.value<long>();
        }

        if ( i != arguments_->size() ) {
          out << ", ";
        }
      }
    }
    else {
      out << "?";
    }

    out << ")";
  }
  else {
    out << "sc-" << syscall_no() << "(?)";
  }

  return out.str();
}

template<>
void SystemCallInvocation::set_argument( uint8_t argnum, const string value )
{
  TracedProcess::set_syscall_arg( pid_, argnum, value );

  if ( arguments_.initialized() ) {
    arguments_->at( argnum ).set_value( value ); /* XXX what about the long_val? */
  }
}

template<>
void SystemCallInvocation::set_argument( uint8_t argnum, const char * value )
{
  string value_string( value );
  set_argument( argnum, value_string );
}

template void SystemCallInvocation::set_argument( uint8_t argnum, const string value );
