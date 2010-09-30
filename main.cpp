////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <SFML/System.hpp>
#include <iostream>

#include "Server.hpp"

////////////////////////////////////////////////////////////
/// A custom class defining insertion / extraction
/// into / from packets
///
////////////////////////////////////////////////////////////
struct Character
{
    sf::Uint16   Age;
    std::string Name;
    float       Height;
};

sf::Packet& operator <<(sf::Packet& Packet, const Character& C)
{
    return Packet << C.Age << C.Name << C.Height;
}

sf::Packet& operator >>(sf::Packet& Packet, Character& C)
{
    return Packet >> C.Age >> C.Name >> C.Height;
}


////////////////////////////////////////////////////////////
/// Our custom packet type, handling encryption and
/// decryption of data
///
////////////////////////////////////////////////////////////
class MyEncryptedPacket : public sf::Packet
{
	private :
	
    ////////////////////////////////////////////////////////////
    /// Called before the packet is sent to the network
    ///
    /// \param DataSize : Variable to fill with the size of data to send
    ///
    /// \return Pointer to the array of bytes to send
    ///
    ////////////////////////////////////////////////////////////
    virtual const char* OnSend(std::size_t& DataSize)
    {
        // Copy the internal data of the packet into our destination buffer
        myBuffer.assign(GetData(), GetData() + GetDataSize());
		
        // Encrypt (powerful algorithm : add 1 to each character !)
        for (std::vector<char>::iterator i = myBuffer.begin(); i != myBuffer.end(); ++i)
            *i += 1;
		
        // Return the size of encrypted data, and a pointer to the buffer containing it
        DataSize = myBuffer.size();
        return &myBuffer[0];
    }
	
    ////////////////////////////////////////////////////////////
    /// Called after the packet has been received from the network
    ///
    /// \param Data :     Pointer to the array of received bytes
    /// \param DataSize : Size of the array of bytes
    ///
    ////////////////////////////////////////////////////////////
    virtual void OnReceive(const char* Data, std::size_t DataSize)
    {
        // Copy the received data into our destination buffer
        myBuffer.assign(Data, Data + DataSize);
		
        // Decrypt data using our powerful algorithm
        for (std::vector<char>::iterator i = myBuffer.begin(); i != myBuffer.end(); ++i)
            *i -= 1;
		
        // Fill the packet with the decrypted data
        Append(&myBuffer[0], myBuffer.size());
    }
	
    std::vector<char> myBuffer;
};


////////////////////////////////////////////////////////////
/// Create a client and connect it to a running server
///
////////////////////////////////////////////////////////////
void RunClient(unsigned short Port)
{
    // Ask for server address
    sf::IPAddress ServerAddress;
    do
    {
        std::cout << "Type address or name of the server to connect to : ";
        std::cin  >> ServerAddress;
    }
    while (!ServerAddress.IsValid());
	
    // Create a TCP socket for communicating with server
    sf::SocketTCP Client;
	
    // Connect to the specified server
    if (Client.Connect(Port, ServerAddress) != sf::Socket::Done)
        return;
    std::cout << "Connected to server " << ServerAddress << std::endl;
	
    /* ----- REGULAR PACKET ----- */
    sf::Packet RegularPacket;
    if (Client.Receive(RegularPacket) != sf::Socket::Done)
        return;
	
    Character C1;
    if (RegularPacket >> C1)
    {
        std::cout << "Character received from the server (regular packet) : " << std::endl;
        std::cout << C1.Name << ", " << C1.Age << " years old, " << C1.Height << " meters" << std::endl;
    }
    /* ----- REGULAR PACKET ----- */
	
    /* ----- ENCRYPTED PACKET ----- */
    MyEncryptedPacket EncryptedPacket;
    if (Client.Receive(EncryptedPacket) != sf::Socket::Done)
        return;
	
    Character C2;
    if (EncryptedPacket >> C2)
    {
        std::cout << "Character received from the server (encrypted packet) : " << std::endl;
        std::cout << C2.Name << ", " << C2.Age << " years old, " << C2.Height << " meters" << std::endl;
    }
    /* ----- ENCRYPTED PACKET ----- */
	
    // Close the socket when we're done
    Client.Close();
}


////////////////////////////////////////////////////////////
/// Launch a server and wait for incoming connections
///
////////////////////////////////////////////////////////////
void RunServer(unsigned short Port)
{
    // Create a TCP socket for communicating with clients
    sf::SocketTCP Server;
	
    // Listen to a port for incoming connections
    if (!Server.Listen(Port))
        return;
    std::cout << "Server is listening to port " << Port << ", waiting for connections... " << std::endl;
	
    // Wait for a connection
    sf::IPAddress ClientAddress;
    sf::SocketTCP Client;
    Server.Accept(Client, &ClientAddress);
    std::cout << "Client connected : " << ClientAddress << std::endl;
	
    /* ----- REGULAR PACKET ----- */
    Character C1 = {12, "Bill", 1.32f};
    sf::Packet RegularPacket;
    RegularPacket << C1;
    if (Client.Send(RegularPacket) != sf::Socket::Done)
        return;
	
    std::cout << "Character sent to the client (regular packet) : " << std::endl;
    std::cout << C1.Name << ", " << C1.Age << " years old, " << C1.Height << " meters" << std::endl;
    /* ----- REGULAR PACKET ----- */
	
    /* ----- ENCRYPTED PACKET ----- */
    Character C2 = {56, "Bob", 1.87f};
    MyEncryptedPacket EncryptedPacket;
    EncryptedPacket << C2;
    if (Client.Send(EncryptedPacket) != sf::Socket::Done)
        return;
	
    std::cout << "Character sent to the client (encrypted packet) : " << std::endl;
    std::cout << C2.Name << ", " << C2.Age << " years old, " << C2.Height << " meters" << std::endl;
    /* ----- ENCRYPTED PACKET ----- */
	
    // Close the sockets when we're done
    Client.Close();
    Server.Close();
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose a random port for opening sockets (ports < 1024 are reserved)
    const unsigned short Port = 2435;
	
    // Client or server ?
    char Who;
    std::cout << "Do you want to be the server ('s') or a client ('c') ? ";
    std::cin  >> Who;
	
    if (Who == 's')
        RunServer(Port);
    else
        RunClient(Port);
	
    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
    std::cin.ignore(10000, '\n');
	
    return EXIT_SUCCESS;
}