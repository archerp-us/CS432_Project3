/**
 * @file p3-analysis.c
 * @brief Compiler phase 3: static analysis
 * @author Philip Archer and Xinzhe He
 */
#include "p3-analysis.h"

//global variable for function return type
DecafType return_type;

/**
 * @brief State/data for static analysis visitor
 */
typedef struct AnalysisData
{
    /**
     * @brief List of errors detected
     */
    ErrorList* errors;

    /* BOILERPLATE: TODO: add any new desired state information (and clean it up in AnalysisData_free) */

} AnalysisData;

/**
 * @brief Allocate memory for analysis data
 * 
 * @returns Pointer to allocated structure
 */
AnalysisData* AnalysisData_new ()
{
    AnalysisData* data = (AnalysisData*)calloc(1, sizeof(AnalysisData));
    CHECK_MALLOC_PTR(data);
    data->errors = ErrorList_new();
    return data;
}

/**
 * @brief Deallocate memory for analysis data
 * 
 * @param data Pointer to the structure to be deallocated
 */
void AnalysisData_free (AnalysisData* data)
{
    /* free everything in data that is allocated on the heap except the error
     * list; it needs to be returned after the analysis is complete */

    /* free "data" itself */
    free(data);
}

/**
 * @brief Macro for more convenient access to the error list inside a @c visitor
 * data structure
 */
#define ERROR_LIST (((AnalysisData*)visitor->data)->errors)

/**
 * @brief Wrapper for @ref lookup_symbol that reports an error if the symbol isn't found
 * 
 * @param visitor Visitor with the error list for reporting
 * @param node AST node to begin the search at
 * @param name Name of symbol to find
 * @returns The @ref Symbol if found, otherwise @c NULL
 */
Symbol* lookup_symbol_with_reporting(NodeVisitor* visitor, ASTNode* node, const char* name)
{
    Symbol* symbol = lookup_symbol(node, name);
    if (symbol == NULL) {
        ErrorList_printf(ERROR_LIST, "Symbol '%s' undefined on line %d", name, node->source_line);
    }
    return symbol;
}

/**
 * @brief Macro for shorter storing of the inferred @c type attribute
 */
#define SET_INFERRED_TYPE(T) ASTNode_set_printable_attribute(node, "type", (void*)(T), \
                                 type_attr_print, dummy_free)

/**
 * @brief Macro for shorter retrieval of the inferred @c type attribute
 */
#define GET_INFERRED_TYPE(N) (DecafType)ASTNode_get_attribute(N, "type")

//Solution
void AnalysisVisitor_check_vardecl (NodeVisitor* visitor, ASTNode* node)
{
	if (node->vardecl.type == VOID)
	{
		ErrorList_printf(ERROR_LIST, "Void variable '%s' on line %d",
			node->vardecl.name, node->source_line);
	}
}

void AnalysisVisitor_check_location (NodeVisitor* visitor, ASTNode* node)
{
	lookup_symbol_with_reporting(visitor, node, node->location.name);
}

void AnalysisVisitor_check_main (NodeVisitor* visitor, ASTNode* node)
{
    Symbol* symbol = lookup_symbol_with_reporting(visitor, node, "main");
    if (symbol == NULL) {
        ErrorList_printf(ERROR_LIST, "Main function undefined on line %d", node->source_line);
    }
    else {
        if (symbol->type != INT || node->funcdecl.parameters != NULL) {
            ErrorList_printf(ERROR_LIST, "Main function defined incorrect on line %d", node->source_line);
        }
    }
}

void AnalysisVisitor_check_conditional (NodeVisitor* visitor, ASTNode* node)
{
	ASTNode* node2 = node->conditional.condition;
	if (node2->vardecl.type != BOOL)
	{
		ErrorList_printf(ERROR_LIST, "Condition must be a boolean type on line %d", node->source_line);
	}
}

void AnalysisVisitor_check_while (NodeVisitor* visitor, ASTNode* node)
{
	ASTNode* node2 = node->whileloop.condition;
	if (node2->vardecl.type != BOOL)
	{
		ErrorList_printf(ERROR_LIST, "Condition must be a boolean type on line %d", node->source_line);
	}
}

// if there is no loops there can not be a break or continue
// UPDATE for edge case where there is loop but break or continue is outside it
void AnalysisVisitor_check_break_continue (NodeVisitor* visitor, ASTNode* node)
{
	if ((lookup_symbol_with_reporting(visitor, node, "if") != NULL) 
		|| (lookup_symbol_with_reporting(visitor, node, "while") != NULL))
	{
	
	}
	else
	{
		ErrorList_printf(ERROR_LIST, "Break on line %d can not exist outside of a loop", node->source_line);
	}
}

void AnalysisVisitor_check_return (NodeVisitor* visitor, ASTNode* node)
{
	if (node->funcreturn.value->vardecl.type != return_type)
	{
		ErrorList_printf(ERROR_LIST, "Return type on line %d does not match function return type", node->source_line);
	}
}

/*void AnalysisVisitor_check_block (NodeVisitor* visitor, ASTNode* node, DecafType return_type)
{

}*/

void AnalysisVisitor_check_funcdecl (NodeVisitor* visitor, ASTNode* node)
{
	//check parameters - do this in previsit

	return_type = node->funcdecl.return_type;
	
	//check function block - might need this
	//AnalysisVisitor_check_block (visitor, node->funcdecl.body, node->funcdecl.return_type);
}

/*void AnalysisVisitor_check_assignment (NodeVisitor* visitor, ASTNode* node)
{
	ASTNode* location = node->assignment.location;
	ASTNode* value = node->assignment.value;
	if (location->vardecl.type != value->literal.type)
	{
		//ErrorList_printf(ERROR_LIST, "Variable '%s' type mismatch on line %d",
			//node->vardecl.name, node->source_line);
		ErrorList_printf(ERROR_LIST, "Variable type mismatch on line %d", node->source_line);
	}
}*/
//

ErrorList* analyze (ASTNode* tree)
{
	// Solution
	if (tree == NULL)
	{
		return ErrorList_new();
	}
	//
	
    /* allocate analysis structures */
    NodeVisitor* v = NodeVisitor_new();
    v->data = (void*)AnalysisData_new();
    v->dtor = (Destructor)AnalysisData_free;

    /* BOILERPLATE: TODO: register analysis callbacks */
	// Solution
		v->postvisit_vardecl = AnalysisVisitor_check_vardecl;
		v->postvisit_location = AnalysisVisitor_check_location;
		v->postvisit_program = AnalysisVisitor_check_main;
		v->postvisit_conditional = AnalysisVisitor_check_conditional;
		v->postvisit_whileloop = AnalysisVisitor_check_while;
		v->postvisit_break = AnalysisVisitor_check_break_continue;
		v->postvisit_continue = AnalysisVisitor_check_break_continue;
		v->postvisit_return = AnalysisVisitor_check_return;
		//v->postvisit_assignment = AnalysisVisitor_check_assignment;
		//v->postvisit_funcdecl = AnalysisVisitor_check_funcdecl;
	//
	
    /* perform analysis, save error list, clean up, and return errors */
    NodeVisitor_traverse(v, tree);
    ErrorList* errors = ((AnalysisData*)v->data)->errors;
    NodeVisitor_free(v);
    return errors;
}
