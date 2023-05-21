#include "MBCC_LSP.h"
int main(int argc,char** argv)
{
    std::unique_ptr<MBLSP::LSP_Server> MBSpecServer = std::make_unique<MBCC::MBCC_LSP>();
    std::unique_ptr<MBLSP::TerminalInput> InputStream = std::make_unique<MBLSP::TerminalInput>(&std::cin);
    std::unique_ptr<MBUtility::MBOctetOutputStream> OutStream = std::make_unique<MBUtility::TerminalOutput>();
    //std::this_thread::sleep_for(10s);
    MBLSP::LSP_ServerHandler Handler(std::move(InputStream),std::move(OutStream),std::move(MBSpecServer));
    Handler.Run();
}
