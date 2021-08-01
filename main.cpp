#include "ContextBuilderModule.h"
#include "Expr.h"
#include "parser.h"


int main() {
    auto cbm = createContextBuilderModule();
    auto machine = cbm.init(2);
    auto expr = parse(cbm,
                      "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.");
//    auto expr = parse(cbm, ",^x>x,-x^y>y,+y.");
    expr->generate(machine);
    cbm.finalizeAndPrintIRtoFile("/home/valerij/test.ll");
    return 0;
}
