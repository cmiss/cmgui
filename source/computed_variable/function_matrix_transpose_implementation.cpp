//******************************************************************************
// FILE : function_matrix_transpose_implementation.cpp
//
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_matrix_transpose.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_specific.hpp"


// module classes
// ==============

// class Function_variable_matrix_transpose
// ----------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_transpose :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_transpose(
			const boost::intrusive_ptr< Function_matrix_transpose<Value_type> >
			function_matrix_transpose):
			Function_variable_matrix<Value_type>(function_matrix_transpose){};
		Function_variable_matrix_transpose(
			const boost::intrusive_ptr< Function_matrix_transpose<Value_type> >
			function_matrix_transpose,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix_transpose,row,column){};
		~Function_variable_matrix_transpose(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_transpose<Value_type>(*this)));
		};
		Function_handle evaluate()
		{
			Function_handle result(0);
			boost::intrusive_ptr< Function_matrix_transpose<Value_type> >
				function_matrix_transpose;

			if (function_matrix_transpose=boost::dynamic_pointer_cast<
				Function_matrix_transpose<Value_type>,Function>(function()))
			{
				Function_size_type number_of_columns,number_of_rows;
				boost::intrusive_ptr< Function_matrix<Value_type> > matrix;

				if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_matrix_transpose->matrix_private->evaluate()))&&
					(row_private<=(number_of_rows=matrix->number_of_columns()))&&
					(column_private<=(number_of_columns=matrix->number_of_rows())))
				{
					Function_size_type i,j;

					function_matrix_transpose->values.resize(number_of_rows,
						number_of_columns);
					for (i=1;i<=number_of_rows;i++)
					{
						for (j=1;j<=number_of_columns;j++)
						{
							function_matrix_transpose->values(i-1,j-1)=(*matrix)(j,i);
						}
					}
					if (0==row_private)
					{
						if (0==column_private)
						{
							result=Function_handle(new Function_matrix<Value_type>(
								function_matrix_transpose->values));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(number_of_rows,1);

							for (i=0;i<number_of_rows;i++)
							{
								result_matrix(i,0)=(function_matrix_transpose->values)(
									i,column_private-1);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
					else
					{
						if (0==column_private)
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,number_of_columns);

							for (j=0;j<number_of_columns;j++)
							{
								result_matrix(0,j)=(function_matrix_transpose->values)(
									row_private-1,j);
							}
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
						else
						{
							ublas::matrix<Value_type,ublas::column_major>
								result_matrix(1,1);
							
							result_matrix(0,0)=(function_matrix_transpose->values)(
								row_private-1,column_private-1);
							result=Function_handle(new Function_matrix<Value_type>(
								result_matrix));
						}
					}
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(std::list<Function_variable_handle>&)
		{
			return (0);
		};
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix_transpose<Value_type> >
				function_matrix_transpose;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix_transpose=boost::dynamic_pointer_cast<
				Function_matrix_transpose<Value_type>,Function>(function_private))&&
				(row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_transpose<Value_type>(
					function_matrix_transpose,row,column));
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_matrix_transpose(
			const Function_variable_matrix_transpose<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


// global classes
// ==============

// class Function_matrix_transpose
// -------------------------------

EXPORT template<typename Value_type>
ublas::matrix<Value_type,ublas::column_major> 
	Function_matrix_transpose<Value_type>::constructor_values(0,0);

EXPORT template<typename Value_type>
Function_matrix_transpose<Value_type>::Function_matrix_transpose(
	const Function_variable_handle& matrix):Function_matrix<Value_type>(
	Function_matrix_transpose<Value_type>::constructor_values),
	matrix_private(matrix){}
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix_transpose<Value_type>::~Function_matrix_transpose()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
string_handle Function_matrix_transpose<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (matrix_private)
		{
			out << "transpose(";
			out << *(matrix_private->get_string_representation());
			out << ")";
		}
		else
		{
			out << "Invalid Function_matrix_transpose";
		}
		*return_string=out.str();
	}

	return (return_string);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_transpose<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function;
	Function_variable_handle result(0);

	if (matrix_private&&(function=matrix_private->function()))
	{
		result=function->input();
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix_transpose<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_transpose<Value_type>(
		boost::intrusive_ptr< Function_matrix_transpose<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix_transpose<Value_type>::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		try
		{
			const Function_matrix_transpose<Value_type>& function_matrix_transpose=
				dynamic_cast<const Function_matrix_transpose<Value_type>&>(function);

			result=
				equivalent(matrix_private,function_matrix_transpose.matrix_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_transpose<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_variable_matrix_transpose<Value_type> >
		atomic_variable_matrix_transpose;
	Function_handle result(0);

	if ((atomic_variable_matrix_transpose=boost::dynamic_pointer_cast<
		Function_variable_matrix_transpose<Value_type>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_transpose->function())&&
		(0<atomic_variable_matrix_transpose->row())&&
		(0<atomic_variable_matrix_transpose->column()))
	{
		result=atomic_variable_matrix_transpose->evaluate();
	}

	return (result);
}

EXPORT template<typename Value_type>
bool Function_matrix_transpose<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

#if !defined (AIX)
template<>
bool Function_matrix_transpose<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // !defined (AIX)

EXPORT template<typename Value_type>
bool Function_matrix_transpose<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_transpose<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;
	Function_handle function;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_transpose<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row())-1,
			(atomic_matrix_variable->column())-1),atomic_value);
	}
	if (!result)
	{
		if (function=matrix_private->function())
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix_transpose<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result(0);
	boost::intrusive_ptr< Function_variable_matrix_transpose<Value_type> >
		atomic_variable_matrix_transpose;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_transpose=boost::dynamic_pointer_cast<
		Function_variable_matrix_transpose<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_transpose->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}
	if (!result)
	{
		if (function=matrix_private->function())
		{
			result=function->get_value(atomic_variable);
		}
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix_transpose<Value_type>::Function_matrix_transpose(
	const Function_matrix_transpose<Value_type>& function_matrix_transpose):
	Function_matrix<Value_type>(function_matrix_transpose),
	matrix_private(function_matrix_transpose.matrix_private){}
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix_transpose<Value_type>&
	Function_matrix_transpose<Value_type>::operator=(
	const Function_matrix_transpose<Value_type>& function_matrix_transpose)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->matrix_private=function_matrix_transpose.matrix_private;
	this->values=function_matrix_transpose.values;

	return (*this);
}
