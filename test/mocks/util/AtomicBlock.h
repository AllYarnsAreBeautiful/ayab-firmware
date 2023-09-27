/*
	AtomicBlock.h

		This is a group of classes designed to replace the functionality of ATOMIC_BLOCK
		with a more main stream approach away from its non C++ standard looking code.

		The system works by simply declaring a variable. The objects have no user callable
		methods, just side effects. Effectively the atomic operation lasts for the lifetime
		of the variable.

		Designed and implemented by Christopher Andrews.

		Distributed under GNU GPL V3 for free software.
		http://www.gnu.org/licenses/gpl.txt

		For more information you can ask questions here: http://arduino.cc/forum/index.php/topic,125253.msg941527.html#msg941527

		Declarable objects.

			- AtomicBlock
				This will turn off interrupts when created. On destruction its operation depends on the passed template parameter.

			- NonAtomicBlock
				This will turn on interrupts when created. On destruction its operation depends on the passed template parameter.

		Passable parameters.

			- Atomic_RestoreState
				When the owning object is destroyed, this mode specifies the state of the global interrupt flag will be returned
				to its original value as the owning object goes out of scope.

			- Atomic_Force
				When the owning object is destroyed, this mode specifies the state of the global interrupt flag will be forced
				to the opposite value set by the owning object when created.

		Optional parameters.

			- Safe operation for Atomic_RestoreState can be gained by using a second template parameter '_Safe' or by using
			  declarable objects suffixed with 'Safe'

		Usage type 1.

			- ATOMIC_BLOCK / NONATOMIC_BLOCK equivalent.

				AtomicBlock< Atomic_RestoreState > 		a_Block;
				NonAtomicBlock< Atomic_RestoreState > 	a_NoBlock;
				AtomicBlock< Atomic_Force > 			a_Block;
				NonAtomicBlock< Atomic_Force > 			a_NoBlock;

			- Status register safe mode.

				AtomicBlock< Atomic_RestoreState, _Safe > 		a_Block;
				NonAtomicBlock< Atomic_RestoreState, _Safe > 	n_NoBlock;

				AtomicBlockSafe< Atomic_RestoreState > 			a_BlockSafe;
				NonAtomicBlockSafe< Atomic_RestoreState > 		n_NoBlockSafe;

				AtomicBlock< Atomic_Force, _Safe > 				a_Block;
				NonAtomicBlock< Atomic_Force, _Safe > 			n_NoBlock;

				AtomicBlockSafe< Atomic_Force > 				a_BlockSafe;
				NonAtomicBlockSafe< Atomic_Force > 				n_NoBlockSafe;

			- Sample usage.

				ISR(TIMER0_COMPA_vect)
					{
						AtomicBlock< Atomic_RestoreState > 	a_Block;

						//Read some data.
						//...
						{
							NonAtomicBlock< Atomic_Force > 	a_NoBlock;

							//Calculate new data
							//...
						}
						//Write some data
					}


			CAUTION: 	As the atomic operation lasts for the lifetime of the variable, anything
						before the declaration will not be protected. Use extra scope operators
						 '{' and '}' to help make things clearer if needed.

		Usage type 2

			Each blocking type contains a function named 'Protect'. It can be used to
			protect any kind of element.

			E.g.

				- Typedefs make using 'Protect' easier.
					typedef NonAtomicBlock< Atomic_Force > MyBlock;

				For the sake of these examples 'i' is an 'int'.

				- Protected writes
					MyBlock::Protect( i ) = analogRead( A0 );

				- Protected reads
					Serial.println( MyBlock::Protect( i ) );

				- Protected non-member function calls.
					MyBlock::Protect( inc )( i );

				- Protected pointers
					*( &MyBlock::Protect( i ) ) = 77;
					(*MyBlock::Protect( &i ))++;

				- No unnessecary instructions.

					- This will not produce any code.

						MyBlock::Protect( 1 + 2 + 3 + 4 + 5 );

					- These two lines produce exactly the same code.

						digitalWrite( MyBlock::Protect( 13 ), digitalRead( MyBlock::Protect( 13 ) ) ^ 1 );
						digitalWrite( 13, digitalRead( 13 ) ^ 1 );

					- This will only turn interrupts on and off once.

						MyBlock::Protect( MyBlock::Protect( MyBlock::Protect( MyBlock::Protect( i ) ) ) );


			CAUTION: 'Protect' will only protect one element. Statements as arguments are not going to work as expected.
				E.g.

					- Wrong use. ( argument statement will not be blocked. If you use the result, it will be inside the atomic block. )
						MyBlock::Protect( PORTK |= _BV( 5 ) );

					- Correct usage. ( just 'Protect' PORTK )
						MyBlock::Protect( PORTK ) |= _BV( 5 );

			LIMITATIONS:

					* I have chosen not to support any sort of member function protection. Once I have validated the current system
					I can look further into it. The required interface seems to generalise the type system too much and breaks some
					existing functionality as the compiler cannot disambiguate the control paths.

		Version history
			- 1.1
				Added 'Protect' method.
			- 1.0
				Safe mode added into  Atomic_RestoreState
			- 0.1
				Initial design
*/


#ifndef HEADER_ATOMICBLOCK
	#define HEADER_ATOMICBLOCK

	#include <avr/io.h>

	#if __cplusplus > 0

		static const bool _Safe = true;

		//This namespace is not for general use.
		namespace{
			inline static void GI_cli( void )	{  __asm__ __volatile__ ("cli" ::); }
			inline static void GI_sei( void )	{  __asm__ __volatile__ ("sei" ::); }

			/*********************************************************************
				_AtomicWrapper interface.
					Provides a temporary object for 'Protect' function.
					Its '_BlockType' is effective throughout its life/scope.
			*********************************************************************/

			template< typename _BlockType, typename _ContainedType >
				class _AtomicWrapper{
					protected:
						template< typename > friend class _AtomicInline;
						_AtomicWrapper( _ContainedType &o_OwningObject ) : b_Block(), c_Object( o_OwningObject ) { return; }
					public:

						operator _ContainedType&()			{ return this->c_Object; }
						operator _ContainedType&() const 	{ return this->c_Object; }

						template< typename _Type >
							_AtomicWrapper< _BlockType, _ContainedType > &operator =( _Type const &t_Copy )
								{
									this->c_Object = t_Copy;
									return *this;
								}

					private:
						_BlockType		b_Block;
						_ContainedType	&c_Object;
			};

			/*********************************************************************
				_TypeSeparator interface.
					Provides instantiation objects for 'Protect'.
			*********************************************************************/

			template< typename _BlockType, typename _Type >
				struct _TypeSeparator									{ typedef _AtomicWrapper< _BlockType, _Type > TemporaryType; };

			template< typename _BlockType, typename _Type >
				struct _TypeSeparator< _BlockType, volatile _Type& >	{ typedef _AtomicWrapper< _BlockType, volatile _Type > TemporaryType; };

			template< typename _BlockType, typename _Type >
				struct _TypeSeparator< _BlockType, const _Type >		{ typedef const _Type& TemporaryType; };


			/*********************************************************************
				_AtomicInline interface.
					Provider of 'Protect' function.
			*********************************************************************/

			template< typename _BlockType >
				class _AtomicInline{
					public:
						template< typename _Type >
							inline static typename _TypeSeparator< _BlockType, _Type >::TemporaryType Protect( _Type &t_Var )
								{ return typename _TypeSeparator< _BlockType, _Type >::TemporaryType( t_Var ); }

						template< typename _Type >
							inline static typename _TypeSeparator< _BlockType, const _Type >::TemporaryType Protect( _Type const &t_Var )
								{ return typename _TypeSeparator< _BlockType, const _Type >::TemporaryType( t_Var ); }
					protected:
						template< template< bool, bool > class, bool > 	friend class AtomicBlock;
						template< template< bool, bool > class, bool > 	friend class NonAtomicBlock;
						template< template< bool, bool > class > 		friend class AtomicBlockSafe;
						template< template< bool, bool > class > 		friend class NonAtomicBlockSafe;
					private:
						_AtomicInline( void ) { return; }
						~_AtomicInline( void ) { return; }
			};

		};

		/*********************************************************************
			Atomic_RestoreState interface.
				This will cause SREG to be returned to its original value
				at the end of its life. Its function depends on '_Atomic'.
		*********************************************************************/

		template< bool _Atomic, bool _SafeRestore = false >
			struct Atomic_RestoreState{

				Atomic_RestoreState( void ) : u_SREG( SREG ) { ( ( _Atomic ? GI_cli : GI_sei ) )(); }

				~Atomic_RestoreState( void )
					{
						if( _SafeRestore ){
							if( _Atomic && ( this->u_SREG & _BV( SREG_I ) ) ) GI_sei();
							if( !_Atomic && !( this->u_SREG & _BV( SREG_I ) ) ) GI_cli();
						}else
							SREG = this->u_SREG;
					}

				const uint8_t u_SREG;
		};

		/*********************************************************************
			Atomic_Force interface.
				This will force SREG to be a state depending on'_Atomic'.
		*********************************************************************/

		template< bool _Atomic, bool _Unused = true >
			struct Atomic_Force{
				Atomic_Force( void ) 	{ ( _Atomic ? GI_cli : GI_sei )(); }
				~Atomic_Force( void ) 	{ ( _Atomic ? GI_sei : GI_cli )(); }
		};

		template< bool _Unused_A = true, bool _Unused_B = true > struct Atomic_None{};

		/*********************************************************************
			Main user interfaces.
		*********************************************************************/

		template< template< bool, bool > class _AtomicMode, bool _SafeRestore = false >  struct AtomicBlock
			:  	_AtomicMode< true, _SafeRestore >,
				_AtomicInline< AtomicBlock< _AtomicMode, _SafeRestore > >{};

		template< template< bool, bool > class _AtomicMode, bool _SafeRestore = false > struct NonAtomicBlock
			:  	_AtomicMode< false, _SafeRestore >,
				_AtomicInline< NonAtomicBlock< _AtomicMode, _SafeRestore > >{};

		template< template< bool, bool > class _AtomicMode > struct AtomicBlockSafe
			:  	_AtomicMode< true, true >,
				_AtomicInline< AtomicBlockSafe< _AtomicMode > >{};

		template< template< bool, bool > class _AtomicMode > struct NonAtomicBlockSafe
			:  	_AtomicMode< false, true >,
				_AtomicInline< NonAtomicBlockSafe< _AtomicMode > >{};
	#endif
#endif

