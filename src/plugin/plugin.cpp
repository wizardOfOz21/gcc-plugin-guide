#include <iostream>
#include <sstream>
#include <string>

#include <gcc-plugin.h>
#include <plugin-version.h>

#include <coretypes.h>

#include <tree-pass.h>
#include <context.h>
#include <basic-block.h>

#include <tree.h>
#include <tree-ssa-alias.h>
#include <gimple-expr.h>
#include <gimple.h>
#include <gimple-ssa.h>
#include <tree-phinodes.h>
#include <tree-ssa-operands.h>

#include <ssa-iterators.h>
#include <gimple-iterator.h>

#define PREFIX_UNUSED(variable) ((void)variable)

int plugin_is_GPL_compatible = 1;

#define PLUGIN_NAME "phi-debug"
#define PLUGIN_VERSION "1.0.0"
#define PLUGIN_HELP "this plugin shows:\n"         \
                    "* basic blocks;\n"            \
                    "* GIMPLE instructions:\n"     \
                    "  - arithmetic operations;\n" \
                    "  - phi-functions;\n"         \
                    "  - branches;\n"              \
                    "  - memory operations;"

#include "print.hpp"

static struct plugin_info phi_debug_plugin_info = {
    .version = PLUGIN_VERSION,
    .help = PLUGIN_HELP,
};

static const struct pass_data phi_debug_pass_data = {
    .type = GIMPLE_PASS,
    .name = PLUGIN_NAME,
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,

    .properties_required = PROP_gimple_any,
    .properties_provided = 0,
    .properties_destroyed = 0,

    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

struct phi_debug_pass : gimple_opt_pass
{
    phi_debug_pass(gcc::context *ctx) : gimple_opt_pass(phi_debug_pass_data, ctx) {}
    virtual unsigned int execute(function *fun) override;
    virtual phi_debug_pass *clone() override { return this; }
};

#define PRINT(data) std::cout << data << std::endl;
string out_path = "";

string phi_debug_op(enum tree_code code)
{
    std::stringstream ss;
    switch (code)
    {
    case POINTER_PLUS_EXPR:
    case PLUS_EXPR:
        ss << "+";
        break;
    case NEGATE_EXPR:
    case MINUS_EXPR:
        ss << "-";
        break;
    case MULT_EXPR:
        ss << "*";
        break;
    case TRUNC_DIV_EXPR:
    case CEIL_DIV_EXPR:
    case FLOOR_DIV_EXPR:
    case ROUND_DIV_EXPR:
    case EXACT_DIV_EXPR:
    case RDIV_EXPR:
        ss << "/";
        break;
    case LSHIFT_EXPR:
        ss << "<<";
        break;
    case RSHIFT_EXPR:
        ss << ">>";
        break;
    case BIT_IOR_EXPR:
        ss << "|";
        break;
    case BIT_XOR_EXPR:
        ss << "^";
        break;
    case BIT_AND_EXPR:
        ss << "&";
        break;
    case BIT_NOT_EXPR:
        ss << "!";
        break;
    case TRUTH_ANDIF_EXPR:
    case TRUTH_AND_EXPR:
        ss << "&&";
        break;
    case TRUTH_ORIF_EXPR:
    case TRUTH_OR_EXPR:
        ss << "||";
        break;
    case TRUTH_XOR_EXPR:
        ss << "^^";
        break;
    case TRUTH_NOT_EXPR:
        ss << "!";
    case LT_EXPR:
    case UNLT_EXPR:
        ss << "<";
        break;
    case LE_EXPR:
    case UNLE_EXPR:
        ss << "<=";
        break;
    case GT_EXPR:
    case UNGT_EXPR:
        ss << ">";
        break;
    case GE_EXPR:
    case UNGE_EXPR:
        ss << ">=";
        break;
    case EQ_EXPR:
    case UNEQ_EXPR:
        ss << "==";
        break;
    case NE_EXPR:
    case LTGT_EXPR:
        ss << "!=";
        break;
    case UNORDERED_EXPR:
        ss << "unord";
        break;
    case ORDERED_EXPR:
        ss << "ord";
        break;
    default:
        ss << "?(" << code << ")?";
        break;
    }
    return ss.str();
}

string phi_debug_tree(tree t)
{
    std::stringstream ss;
    switch (TREE_CODE(t))
    {
    case INTEGER_CST:
        ss << TREE_INT_CST_LOW(t);
        break;
    case REAL_CST:
    {
        ss << "REAL_CST";
        break;
    }
    case FIXED_CST:
    {
        ss << "FIXED_CST";
        break;
    }
    case COMPLEX_CST:
        ss << "COMPLEX_CST";
        break;
    case VECTOR_CST:
        ss << "VECTOR_CST";
        break;
    case STRING_CST:
        ss << "'" << TREE_STRING_POINTER(t) << "'";
        break;
    case LABEL_DECL:
        ss << (DECL_NAME(t) ? IDENTIFIER_POINTER(DECL_NAME(t)) : "unk_label_decl") << ":";
        break;
    case FIELD_DECL:
        ss << (DECL_NAME(t) ? IDENTIFIER_POINTER(DECL_NAME(t)) : "unk_field_decl");
        break;
    case VAR_DECL:
        ss << (DECL_NAME(t) ? IDENTIFIER_POINTER(DECL_NAME(t)) : "unk_var_decl");
        break;
    case CONST_DECL:
        ss << (DECL_NAME(t) ? IDENTIFIER_POINTER(DECL_NAME(t)) : "unk_const_decl");
        break;
    // https://gcc.gnu.org/onlinedocs/gccint/Storage-References.html
    case ARRAY_REF:
        ss << "ARRAY_REF(";
        ss << phi_debug_tree(TREE_OPERAND(t, 0));
        ss << "[";
        ss << phi_debug_tree(TREE_OPERAND(t, 1));
        ss << "])";
        break;
    case ARRAY_RANGE_REF:
        ss << phi_debug_tree(TREE_OPERAND(t, 0));
        ss << "[";
        ss << phi_debug_tree(TREE_OPERAND(t, 1));
        ss << ":";
        ss << phi_debug_tree(TREE_OPERAND(t, 2));
        ss << "]";
        break;
    case INDIRECT_REF:
        ss << "*";
        ss << phi_debug_tree(TREE_OPERAND(t, 0));
        break;
    case CONSTRUCTOR:
        ss << "constructor";
        break;
    case ADDR_EXPR:
        ss << "&";
        ss << phi_debug_tree(TREE_OPERAND(t, 0));
        break;
    case MEM_REF:
        ss << "((typeof(";
        ss << phi_debug_tree(TREE_OPERAND(t, 1));
        ss << "))";
        ss << phi_debug_tree(TREE_OPERAND(t, 0));
        ss << ")";
        break;
    case SSA_NAME:
    {
        gimple *stmt = SSA_NAME_DEF_STMT(t);
        if (gimple_code(stmt) == GIMPLE_PHI)
        {
            ss << "(" << (SSA_NAME_IDENTIFIER(t) ? IDENTIFIER_POINTER(SSA_NAME_IDENTIFIER(t)) : "unk_ssa_name") << "__v" << SSA_NAME_VERSION(t);
            ss << " = GIMPLE_PHI(";
            for (unsigned int i = 0; i < gimple_phi_num_args(stmt); i++)
            {
                ss << phi_debug_tree(gimple_phi_arg(stmt, i)->def);
                if (i != gimple_phi_num_args(stmt) - 1)
                {
                    ss << ", ";
                }
            }
            ss << "))";
        }
        else
        {
            ss << (SSA_NAME_IDENTIFIER(t) ? IDENTIFIER_POINTER(SSA_NAME_IDENTIFIER(t)) : "unk_ssa_name") << "__v" << SSA_NAME_VERSION(t);
        }

        break;
    }
    default:
        ss << "unk_tree_code(" << TREE_CODE(t) << ")";
        break;
    }

    return ss.str();
}

string phi_debug_on_unknown_stmt(gimple *stmt)
{
    std::stringstream ss;
    ss << "GIMPLE_UNKNOWN";
    return ss.str();
}

string phi_debug_on_gimple_return(gimple *stmt)
{
    std::stringstream ss;
    PREFIX_UNUSED(stmt);
    ss << "GIMPLE_RETURN:  ";
    ss << phi_debug_tree(gimple_return_retval((greturn*)stmt));
    return ss.str();
}

string phi_debug_on_gimple_label(gimple *stmt)
{
    std::stringstream ss;
    PREFIX_UNUSED(stmt);
    ss << "GIMPLE_LABEL:  ";
    ss << phi_debug_tree(gimple_label_label((glabel*)stmt));
    return ss.str();
}

string phi_debug_on_gimple_cond(gimple *stmt)
{
    std::stringstream ss;

    ss << "GIMPLE_COND:  ";
    ss << phi_debug_tree(gimple_cond_lhs(stmt));
    ss << " ";
    ss << phi_debug_op(gimple_assign_rhs_code(stmt));
    ss << " ";
    ss << phi_debug_tree(gimple_cond_rhs(stmt));
    return ss.str();
}

string phi_debug_on_gimple_call(gimple *stmt)
{
    std::stringstream ss;
    ss << "GIMPLE_CALL:  ";
    tree lhs = gimple_call_lhs(stmt);
    if (lhs)
    {
        ss << phi_debug_tree(lhs);
        ss << " = ";
    }
    ss << fndecl_name(gimple_call_fndecl(stmt)) << "(";
    for (unsigned int i = 0; i < gimple_call_num_args(stmt); i++)
    {
        ss << phi_debug_tree(gimple_call_arg(stmt, i));
        if (i != gimple_call_num_args(stmt) - 1)
        {
            ss << ", ";
        }
    }
    ss << ")";

    return ss.str();
}

string phi_debug_on_gimple_assign(gimple *stmt)
{
    std::stringstream ss;
    ss << "GIMPLE_ASSIGN:  ";
    switch (gimple_num_ops(stmt))
    {
    case 2:
        ss << phi_debug_tree(gimple_assign_lhs(stmt));
        ss << " = ";
        ss << phi_debug_tree(gimple_assign_rhs1(stmt));
        break;
    case 3:
        ss << phi_debug_tree(gimple_assign_lhs(stmt));
        ss << " = ";
        ss << phi_debug_tree(gimple_assign_rhs1(stmt));
        ss << " ";
        ss << phi_debug_op(gimple_assign_rhs_code(stmt));
        ss << " ";
        ss << phi_debug_tree(gimple_assign_rhs2(stmt));
        break;
    }

    return ss.str();
}

string phi_debug_statements(basic_block bb)
{
    std::stringstream ss;
    for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
    {
        gimple *stmt = gsi_stmt(gsi);

        switch (gimple_code(stmt))
        {
        case GIMPLE_ASSIGN:
            ss << phi_debug_on_gimple_assign(stmt);
            break;
        case GIMPLE_CALL:
            ss << phi_debug_on_gimple_call(stmt);
            break;
        case GIMPLE_COND:
            ss << phi_debug_on_gimple_cond(stmt);
            break;
        case GIMPLE_LABEL:
            ss << phi_debug_on_gimple_label(stmt);
            break;
        case GIMPLE_RETURN:
            ss << phi_debug_on_gimple_return(stmt);
            break;
        default:
            ss << phi_debug_on_unknown_stmt(stmt);
            break;
        }
        ss << "\n\n";
    }

    return ss.str();
}

std::string phi_debug_bb_id(basic_block bb)
{
    int index = bb->index;

    std::stringstream dst_stream;

    string stmts = phi_debug_statements(bb);
    dst_stream << index << graph_args({graph_arg("label", q(stmts))});

    edge e;
    edge_iterator it;
    FOR_EACH_EDGE(e, it, bb->succs)
    {
        const string succ_name = std::to_string(e->dest->index);
        dst_stream << graph_edge(std::to_string(index), succ_name) << std::endl;
    }
    return dst_stream.str();
}

std::string phi_debug_function(function *fn)
{
    std::stringstream res;

    basic_block bb;
    FOR_EACH_BB_FN(bb, fn)
    {
        res << phi_debug_bb_id(bb) << std::endl;
    }

    return graph(res.str(), string(function_name(fn)));
}

unsigned int phi_debug_pass::execute(function *fn)
{
    string res = phi_debug_function(fn);
    string filename = out_path + "/" + string(function_name(fn));
    PRINT(filename)
    write_to_file(filename, res);
    return 0;
}

static struct register_pass_info phi_debug_pass_info = {
    .pass = new phi_debug_pass(g),
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER, // то есть плагин сработает
                                     // после формирования ssa-формы
};

int plugin_init(struct plugin_name_args *args, struct plugin_gcc_version *version)
{
    if (!plugin_default_version_check(version, &gcc_version))
    {
        return 1;
    }

    out_path = (args->argv)[0].value;

    register_callback(args->base_name, PLUGIN_INFO, NULL, &phi_debug_plugin_info);
    register_callback(args->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &phi_debug_pass_info);

    return 0;
}
