
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
/* $FreeBSD$ */
int destructed;
int destructed2;

class Test {
public:
	Test() { printf("Test::Test()\n"); }
	~Test() { printf("Test::~Test()\n"); destructed = 1; }
};

void
cleanup_handler(void *arg)
{
	destructed2 = 1;
	printf("%s()\n", __func__);
}

void
check_destruct(void)
{
	if (!destructed)
		printf("Bug, object destructor is not called\n");
	else
		printf("OK\n");
}

void
check_destruct2(void)
{
	if (!destructed)
		printf("Bug, object destructor is not called\n");
	else if (!destructed2)
		printf("Bug, cleanup handler is not called\n");
	else
		printf("OK\n");
}