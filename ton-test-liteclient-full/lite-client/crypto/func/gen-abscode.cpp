#include "func.h"

namespace funC {

/*
 * 
 *   EXPRESSIONS
 * 
 */

Expr* Expr::copy() const {
  auto res = new Expr{*this};
  for (auto& arg : res->args) {
    arg = arg->copy();
  }
  return res;
}

Expr::Expr(int c, sym_idx_t name_idx, std::initializer_list<Expr*> _arglist) : cls(c), args(std::move(_arglist)) {
  sym = sym::lookup_symbol(name_idx);
  if (!sym) {
  }
}

void Expr::chk_rvalue(const Lexem& lem) const {
  if (!is_rvalue()) {
    lem.error_at("rvalue expected before `", "`");
  }
}

void Expr::chk_lvalue(const Lexem& lem) const {
  if (!is_lvalue()) {
    lem.error_at("lvalue expected before `", "`");
  }
}

void Expr::chk_type(const Lexem& lem) const {
  if (!is_type()) {
    lem.error_at("type expression expected before `", "`");
  }
}

bool Expr::deduce_type(const Lexem& lem) {
  if (e_type) {
    return true;
  }
  switch (cls) {
    case _Apply: {
      if (!sym) {
        return false;
      }
      SymVal* sym_val = dynamic_cast<SymVal*>(sym->value);
      if (!sym_val || !sym_val->get_type()) {
        return false;
      }
      std::vector<TypeExpr*> arg_types;
      for (const auto& arg : args) {
        arg_types.push_back(arg->e_type);
      }
      TypeExpr* fun_type = TypeExpr::new_map(TypeExpr::new_tensor(arg_types), TypeExpr::new_hole());
      try {
        uniformize(fun_type, sym_val->sym_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "cannot apply function " << sym->name() << " : " << sym_val->get_type() << " to arguments of type "
           << fun_type->args[0] << ": " << ue;
        lem.error(os.str());
      }
      e_type = fun_type->args[1];
      TypeExpr::remove_indirect(e_type);
      return true;
    }
    case _VarApply: {
      assert(args.size() == 2);
      TypeExpr* fun_type = TypeExpr::new_map(args[1]->e_type, TypeExpr::new_hole());
      try {
        uniformize(fun_type, args[0]->e_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "cannot apply expression of type " << args[0]->e_type << " to an expression of type " << args[1]->e_type
           << ": " << ue;
        lem.error(os.str());
      }
      e_type = fun_type->args[1];
      TypeExpr::remove_indirect(e_type);
      return true;
    }
    case _Letop: {
      assert(args.size() == 2);
      try {
        // std::cerr << "in assignment: " << args[0]->e_type << " from " << args[1]->e_type << std::endl;
        uniformize(args[0]->e_type, args[1]->e_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "cannot assign an expression of type " << args[1]->e_type << " to a variable or pattern of type "
           << args[0]->e_type << ": " << ue;
        lem.error(os.str());
      }
      e_type = args[0]->e_type;
      TypeExpr::remove_indirect(e_type);
      return true;
    }
    case _LetFirst: {
      assert(args.size() == 2);
      TypeExpr* rhs_type = TypeExpr::new_tensor({args[0]->e_type, TypeExpr::new_hole()});
      try {
        // std::cerr << "in implicit assignment of a modifying method: " << rhs_type << " and " << args[1]->e_type << std::endl;
        uniformize(rhs_type, args[1]->e_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "cannot implicitly assign an expression of type " << args[1]->e_type
           << " to a variable or pattern of type " << rhs_type << " in modifying method `" << sym::symbols.get_name(val)
           << "` : " << ue;
        lem.error(os.str());
      }
      e_type = rhs_type->args[1];
      TypeExpr::remove_indirect(e_type);
      // std::cerr << "result type is " << e_type << std::endl;
      return true;
    }
    case _CondExpr: {
      assert(args.size() == 3);
      auto flag_type = TypeExpr::new_atomic(_Int);
      try {
        uniformize(args[0]->e_type, flag_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "condition in a conditional expression has non-integer type " << args[0]->e_type << ": " << ue;
        lem.error(os.str());
      }
      try {
        uniformize(args[1]->e_type, args[2]->e_type);
      } catch (UniformizeError& ue) {
        std::ostringstream os;
        os << "the two variants in a conditional expression have different types " << args[1]->e_type << " and "
           << args[2]->e_type << " : " << ue;
        lem.error(os.str());
      }
      e_type = args[1]->e_type;
      TypeExpr::remove_indirect(e_type);
      return true;
    }
  }
  return false;
}

int Expr::define_new_vars(CodeBlob& code) {
  switch (cls) {
    case _Tuple:
    case _TypeApply: {
      int res = 0;
      for (const auto& x : args) {
        res += x->define_new_vars(code);
      }
      return res;
    }
    case _Var:
      if (val < 0) {
        val = code.create_var(TmpVar::_Named, e_type, sym, &here);
        return 1;
      }
      break;
    case _Hole:
      if (val < 0) {
        val = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      }
      break;
  }
  return 0;
}

int Expr::predefine_vars() {
  switch (cls) {
    case _Tuple:
    case _TypeApply: {
      int res = 0;
      for (const auto& x : args) {
        res += x->predefine_vars();
      }
      return res;
    }
    case _Var:
      if (!sym) {
        assert(val < 0 && here.defined());
        sym = sym::define_symbol(~val, false, here);
        if (!sym) {
          throw src::ParseError{here, std::string{"redefined variable `"} + sym::symbols.get_name(~val) + "`"};
        }
        sym->value = new SymVal{SymVal::_Var, -1, e_type};
        return 1;
      }
      break;
  }
  return 0;
}

std::vector<var_idx_t> Expr::pre_compile(CodeBlob& code) const {
  switch (cls) {
    case _Tuple: {
      std::vector<var_idx_t> res;
      for (const auto& x : args) {
        auto add = x->pre_compile(code);
        res.insert(res.end(), add.cbegin(), add.cend());
      }
      return res;
    }
    case _Apply: {
      assert(sym);
      std::vector<var_idx_t> res;
      auto func = dynamic_cast<SymValFunc*>(sym->value);
      if (func && func->arg_order.size() == args.size()) {
        //std::cerr << "!!! reordering " << args.size() << " arguments of " << sym->name() << std::endl;
        std::vector<std::vector<var_idx_t>> add_list(args.size());
        for (int i : func->arg_order) {
          add_list[i] = args[i]->pre_compile(code);
        }
        for (const auto& add : add_list) {
          res.insert(res.end(), add.cbegin(), add.cend());
        }
      } else {
        for (const auto& x : args) {
          auto add = x->pre_compile(code);
          res.insert(res.end(), add.cbegin(), add.cend());
        }
      }
      var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      std::vector<var_idx_t> rvect{rv};
      auto& op = code.emplace_back(here, Op::_Call, rvect, std::move(res), sym);
      if (flags & _IsImpure) {
        op.flags |= Op::_Impure;
      }
      return rvect;
    }
    case _TypeApply:
      return args[0]->pre_compile(code);
    case _Var:
    case _Hole:
      return {val};
    case _VarApply:
      if (args[0]->cls == _Glob) {
        std::vector<var_idx_t> res = args[1]->pre_compile(code);
        var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
        std::vector<var_idx_t> rvect{rv};
        auto& op = code.emplace_back(here, Op::_Call, rvect, std::move(res), args[0]->sym);
        if (args[0]->flags & _IsImpure) {
          op.flags |= Op::_Impure;
        }
        return rvect;
      } else {
        std::vector<var_idx_t> res = args[1]->pre_compile(code);
        std::vector<var_idx_t> tfunc = args[0]->pre_compile(code);
        if (tfunc.size() != 1) {
          throw src::Fatal{"stack tuple used as a function"};
        }
        res.push_back(tfunc[0]);
        var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
        std::vector<var_idx_t> rvect{rv};
        code.emplace_back(here, Op::_CallInd, rvect, std::move(res));
        return rvect;
      }
    case _Const: {
      var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      std::vector<var_idx_t> rvect{rv};
      code.emplace_back(here, Op::_IntConst, rvect, intval);
      return rvect;
    }
    case _Glob: {
      var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      std::vector<var_idx_t> rvect{rv};
      code.emplace_back(here, Op::_GlobVar, rvect, std::vector<var_idx_t>{}, sym);
      return rvect;
    }
    case _Letop: {
      std::vector<var_idx_t> right = args[1]->pre_compile(code);
      std::vector<var_idx_t> left = args[0]->pre_compile(code);
      code.emplace_back(here, Op::_Let, left, std::move(right));
      return left;
    }
    case _LetFirst: {
      var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      std::vector<var_idx_t> right = args[1]->pre_compile(code);
      std::vector<var_idx_t> left = args[0]->pre_compile(code);
      left.push_back(rv);
      code.emplace_back(here, Op::_Let, std::move(left), std::move(right));
      return std::vector<var_idx_t>{rv};
    }
    case _CondExpr: {
      auto cond = args[0]->pre_compile(code);
      assert(cond.size() == 1);
      var_idx_t rv = code.create_var(TmpVar::_Tmp, e_type, nullptr, &here);
      std::vector<var_idx_t> rvect{rv};
      Op& if_op = code.emplace_back(here, Op::_If, cond);
      code.push_set_cur(if_op.block0);
      code.emplace_back(here, Op::_Let, rvect, args[1]->pre_compile(code));
      code.close_pop_cur(args[1]->here);
      code.push_set_cur(if_op.block1);
      code.emplace_back(here, Op::_Let, rvect, args[2]->pre_compile(code));
      code.close_pop_cur(args[2]->here);
      return rvect;
    }
    default:
      std::cerr << "expression constructor is " << cls << std::endl;
      throw src::Fatal{"cannot compile expression with unknown constructor"};
  }
}

}  // namespace funC
