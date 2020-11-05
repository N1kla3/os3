//
// Created by nicolas on 26/10/2020.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <csignal>
#include "TCPSocket.h"

void machineHandler(const TCPSocketPtr& client);
bool writeFile(uint16_t index, float result);

std::atomic_uint16_t counter = 0;
std::atomic_uint16_t curr_counter = 0;
std::mutex counter_mutex;

void ex(int signal)
{
   std::abort();
}

int main()
{
    signal(SIGTERM, ex);
    TCPSocketPtr server = TCPSocket::CreateTCP();
    SocketAddress address(44444, INADDR_ANY);
    if (server->Bind(address) != NO_ERROR)
    {
        return -1;
    }
    while (server->Listen() == NO_ERROR)
    {
        SocketAddress addr;
        auto client = server->Accept(addr);
        if (client)
        {
            std::thread thr(machineHandler, client);
            thr.detach();
        }
    }
    /*std::ofstream ofs;
    ofs.open("../res.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();*/
    return 0;
}

uint16_t getAddCounter()
{
    counter_mutex.lock();
    auto temp = static_cast<uint16_t>(counter);
    counter++;
    counter_mutex.unlock();
    return temp;
}

void machineHandler(const TCPSocketPtr& client)
{
    auto temp = getAddCounter();
    while (client->Send((char*)&temp, 2))
    {
        char buffer[4];
        client->Recieve(buffer, sizeof(float));
        float answer = *reinterpret_cast<float*>(&buffer);
        while (!writeFile(temp, answer)){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        temp = getAddCounter();
    }
}

bool writeFile(uint16_t index, float result)
{
   if (curr_counter == 65535)std::raise(SIGTERM);
   if (index != curr_counter) return false;
   std::ofstream file("../res.txt", std::ios::app);
   if (file.is_open())
   {
       file << index << " " << result << "\n";
   } else std::cout << "FILE PROBLEMS";
   file.close();
   curr_counter++;
   return true;
}