//******************************************************************************
// FILE : function_variable_value_specific.hpp
//
// LAST MODIFIED : 21 July 2004
//
// DESCRIPTION :
// An abstract class for accessing the value of variable.  A mediator which
// allows a variable to set its value from another variable only having to know
// about its value type.
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (__FUNCTION_VARIABLE_VALUE_SPECIFIC_HPP__)
#define __FUNCTION_VARIABLE_VALUE_SPECIFIC_HPP__

#include "computed_variable/function_variable_value.hpp"

EXPORT template<typename Value_type>
class Function_variable_value_specific : public Function_variable_value
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// A variable's specific value type.
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_specific(
			bool (*set_function)(Value_type&,const Function_variable_handle));
		// destructor
		virtual ~Function_variable_value_specific();
		const std::string type();
		bool set(Value_type& value,const Function_variable_handle variable);
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value_specific(
			const Function_variable_value_specific<Value_type>&);
		void operator=(const Function_variable_value_specific<Value_type>&);
	private:
		static const std::string type_string;
		bool (*set_function)(Value_type&,const Function_variable_handle);
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_variable_value_specific_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_SPECIFIC_HPP__) */
