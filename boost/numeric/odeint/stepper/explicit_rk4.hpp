/*
 boost header: NUMERIC_ODEINT_STEPPER/explicit_rk4.hpp

 Copyright 2009 Karsten Ahnert
 Copyright 2009 Mario Mulansky
 Copyright 2009 Andre Bergner

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_NUMERIC_ODEINT_STEPPER_EXPLICIT_RK4_HPP_INCLUDED
#define BOOST_NUMERIC_ODEINT_STEPPER_EXPLICIT_RK4_HPP_INCLUDED

#include <boost/ref.hpp>

#include <boost/numeric/odeint/stepper/base/explicit_stepper_base.hpp>
#include <boost/numeric/odeint/algebra/range_algebra.hpp>
#include <boost/numeric/odeint/algebra/default_operations.hpp>
#include <boost/numeric/odeint/stepper/detail/macros.hpp>

#include <boost/numeric/odeint/util/state_wrapper.hpp>
#include <boost/numeric/odeint/util/resizer.hpp>

namespace boost {
namespace numeric {
namespace odeint {


template<
    class State ,
    class Value = double ,
    class Deriv = State ,
    class Time = Value ,
	class Algebra = range_algebra ,
	class Operations = default_operations ,
	class Resizer = initially_resizer
	>
class explicit_rk4
: public explicit_stepper_base<
	  explicit_rk4< State , Value , Deriv , Time , Algebra , Operations , Resizer > ,
	  4 , State , Value , Deriv , Time , Algebra , Operations , Resizer >
{
/*	void initialize( void )
	{
		boost::numeric::odeint::construct( m_dxt );
		boost::numeric::odeint::construct( m_dxm );
		boost::numeric::odeint::construct( m_dxh );
		boost::numeric::odeint::construct( m_x_tmp );
		m_deriv_adjuster.register_state( 0 , m_dxt );
		m_deriv_adjuster.register_state( 1 , m_dxm );
		m_deriv_adjuster.register_state( 2 , m_dxh );
		m_state_adjuster.register_state( 0 , m_x_tmp );
	}

	void copy( const explicit_rk4 &rk )
	{
		boost::numeric::odeint::copy( rk.m_dxt , m_dxt );
		boost::numeric::odeint::copy( rk.m_dxm , m_dxm );
		boost::numeric::odeint::copy( rk.m_dxh , m_dxh );
		boost::numeric::odeint::copy( rk.m_x_tmp , m_x_tmp );
	}
	*/

public :


	BOOST_ODEINT_EXPLICIT_STEPPERS_TYPEDEFS( explicit_rk4 , 4 );

	typedef explicit_rk4< State , Value , Deriv , Time , Algebra , Operations , Resizer > stepper_type;

/*
	explicit_rk4( void )
	: stepper_base_type() , m_deriv_adjuster() , m_state_adjuster() , m_dxt() , m_dxm() , m_dxh() , m_x_tmp()
	{
		initialize();
	}

	~explicit_rk4( void )
	{
		boost::numeric::odeint::destruct( m_dxt );
		boost::numeric::odeint::destruct( m_dxm );
		boost::numeric::odeint::destruct( m_dxh );
		boost::numeric::odeint::destruct( m_x_tmp );
	}

	explicit_rk4( const explicit_rk4 &rk )
	: stepper_base_type( rk ) , m_deriv_adjuster() , m_state_adjuster() , m_dxt() , m_dxm() , m_dxh() , m_x_tmp()
	{
		initialize();
		copy( rk );
	}

	explicit_rk4& operator=( const explicit_rk4 &rk )
	{
		stepper_base_type::operator=( rk );
		copy( rk );
		return *this;
	}
*/

	template< class System , class StateIn , class DerivIn , class StateOut >
	void do_step_impl( System system , const StateIn &in , const DerivIn &dxdt , const time_type &t , StateOut &out , const time_type &dt )
	{
		// ToDo : check if size of in,dxdt,out are equal?

        const value_type val1 = static_cast< value_type >( 1.0 );

		m_resizer.adjust_size( in , boost::bind( &stepper_type::resize< StateIn > , boost::ref( *this ) , _1 ) );

		typename boost::unwrap_reference< System >::type &sys = system;

        const time_type dh = static_cast< value_type >( 0.5 ) * dt;
        const time_type th = t + dh;

        // dt * dxdt = k1
        // m_x_tmp = x + dh*dxdt
        algebra_type::for_each3( m_x_tmp.m_v , in , dxdt ,
        		typename operations_type::template scale_sum2< value_type , time_type >( val1 , dh ) );


        // dt * m_dxt = k2
        sys( m_x_tmp.m_v , m_dxt.m_v , th );

        // m_x_tmp = x + dh*m_dxt
        algebra_type::for_each3( m_x_tmp.m_v , in , m_dxt.m_v ,
        		typename operations_type::template scale_sum2< value_type , time_type >( val1 , dh ) );


        // dt * m_dxm = k3
        sys( m_x_tmp.m_v , m_dxm.m_v , th );
        //m_x_tmp = x + dt*m_dxm
        algebra_type::for_each3( m_x_tmp.m_v , in , m_dxm.m_v ,
        		typename operations_type::template scale_sum2< value_type , time_type >( val1 , dt ) );


        // dt * m_dxh = k4
        sys( m_x_tmp.m_v , m_dxh.m_v , t + dt );
        //x += dt/6 * ( m_dxdt + m_dxt + val2*m_dxm )
        time_type dt6 = dt / static_cast< value_type >( 6.0 );
        time_type dt3 = dt / static_cast< value_type >( 3.0 );
        algebra_type::for_each6( out , in , dxdt , m_dxt.m_v , m_dxm.m_v , m_dxh.m_v ,
        		typename operations_type::template scale_sum5< value_type , time_type , time_type , time_type , time_type >( 1.0 , dt6 , dt3 , dt3 , dt6 ) );
	}

	template< class StateIn >
	bool resize( const StateIn &x )
	{
	    bool resized = false;
        resized |= adjust_size_by_resizeability( m_x_tmp , x , typename wrapped_state_type::is_resizeable() );
        resized |= adjust_size_by_resizeability( m_dxm , x , typename wrapped_deriv_type::is_resizeable() );
        resized |= adjust_size_by_resizeability( m_dxt , x , typename wrapped_deriv_type::is_resizeable() );
        resized |= adjust_size_by_resizeability( m_dxh , x , typename wrapped_deriv_type::is_resizeable() );
        return resized;
	}

	template< class StateType >
	void adjust_size( const StateType &x )
	{
	    resize( x );
		stepper_base_type::adjust_size( x );
	}


private:

	resizer_type m_resizer;

    wrapped_deriv_type m_dxt;
    wrapped_deriv_type m_dxm;
    wrapped_deriv_type m_dxh;
    wrapped_state_type m_x_tmp;

};




} // odeint
} // numeric
} // boost


#endif //BOOST_NUMERIC_ODEINT_STEPPER_EXPLICIT_RK4_HPP_INCLUDED
