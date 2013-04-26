
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "checking.hh"
#include <stdio.h>

namespace dtc
{
namespace fdt
{
namespace checking
{

bool
checker::visit_node(device_tree *tree, node *n)
{
	path.push_back(std::make_pair(n->name, n->unit_address));
	// Check this node
	if (!check_node(tree, n))
	{
		return false;
	}
	// Now check its properties
	for (node::property_iterator i=n->property_begin(), e=n->property_end()
	     ; i!=e ; ++i)
	{
		if (!check_property(tree, n, *i))
		{
			return false;
		}
	}
	// And then recursively check the children
	for (node::child_iterator i=n->child_begin(), e=n->child_end() ; i!=e ;
	     ++i)
	{
		if (!visit_node(tree, *i))
		{
			return false;
		}
	}
	path.pop_back();
	return true;
}

void
checker::report_error(const char *errmsg)
{
	fprintf(stderr, "Error: %s, while checking node: ", errmsg);
	for (device_tree::node_path::iterator p=path.begin()+1, pe=path.end() ;
	     p!=pe ; ++p)
	{
		putc('/', stderr);
		p->first.dump();
		if (!(p->second.empty()))
		{
			putc('@', stderr);
			p->second.dump();
		}
	}
	fprintf(stderr, " [-W%s]\n", checker_name);
}

bool
property_checker::check_property(device_tree *tree, node *n, property *p)
{
	if (p->get_key() == key)
	{
		if (!check(tree, n, p))
		{
			report_error("property check failed");
			return false;
		}
	}
	return true;
}

bool
property_size_checker::check(device_tree *tree, node *n, property *p)
{
	uint32_t psize = 0;
	for (property::value_iterator i=p->begin(),e=p->end() ; i!=e ; ++i)
	{
		if (!i->is_binary())
		{
			return false;
		}
		psize += i->byte_data.size();
	}
	return psize == size;
}

template<property_value::value_type T>
void
check_manager::add_property_type_checker(const char *name, string prop)
{
	checkers.insert(std::make_pair(string(name),
		new property_type_checker<T>(name, prop)));
}

void
check_manager::add_property_size_checker(const char *name,
                                         string prop,
                                         uint32_t size)
{
	checkers.insert(std::make_pair(string(name),
		new property_size_checker(name, prop, size)));
}

check_manager::~check_manager()
{
	while (checkers.begin() != checkers.end())
	{
		delete checkers.begin()->second;
		checkers.erase(checkers.begin());
	}
	while (disabled_checkers.begin() != disabled_checkers.end())
	{
		delete disabled_checkers.begin()->second;
	}
}

check_manager::check_manager()
{
	// NOTE: All checks listed here MUST have a corresponding line
	// in the man page!
	add_property_type_checker<property_value::STRING_LIST>(
			"type-compatible", string("compatible"));
	add_property_type_checker<property_value::STRING>(
			"type-model", string("model"));
	add_property_size_checker("type-phandle", string("phandle"), 4);
}

bool
check_manager::run_checks(device_tree *tree, bool keep_going)
{
	bool success = true;
	for (std::map<string, checker*>::iterator i=checkers.begin(),
	     e=checkers.end() ; i!=e ; ++i)
	{
		success &= i->second->check_tree(tree);
		if (!(success || keep_going))
		{
			break;
		}
	}
	return success;
}

bool
check_manager::disable_checker(string name)
{
	std::map<string, checker*>::iterator checker = checkers.find(name);
	if (checker != checkers.end())
	{
		disabled_checkers.insert(std::make_pair(name,
		                                        checker->second));
		checkers.erase(checker);
		return true;
	}
	return false;
}

bool
check_manager::enable_checker(string name)
{
	std::map<string, checker*>::iterator checker =
		disabled_checkers.find(name);
	if (checker != disabled_checkers.end())
	{
		checkers.insert(std::make_pair(name, checker->second));
		disabled_checkers.erase(checker);
		return true;
	}
	return false;
}

} // namespace checking

} // namespace fdt

} // namespace dtc