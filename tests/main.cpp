void test_mesh();
void test_eangles();
void test_matrix();
void test_toys();

#include <tinker/shell.h>

class CTester : public CShell
{
public:
	CTester(int argc, char** args) : CShell(argc, args) {};
};

int main(int argc, char** args)
{
	CTester c(argc, args);

	test_mesh();
	test_eangles();
	test_matrix();
	test_toys();
}
