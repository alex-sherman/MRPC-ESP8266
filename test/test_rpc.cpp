#include "mrpc.h"

using namespace MRPC;

int main() {
    Transport *trans = new SocketTransport(50123);
    Node *self = Node::Single();
    self->use_transport(trans);
    self->wait();
}