#include <iostream>
#include <fstream>
#include <vector>
#include <malloc.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdint>
#include <bitset>
#include <windows.h>
#include "src/PassiveSocket.h"
#include <stdio.h>
using namespace std;

///         functions declarations
//////////////////////////////////////////////////////////////
void get_API_version (CActiveSocket& SocketActive);

void get_sw_revision(CActiveSocket& SocketActive);

void get_gru_state(CActiveSocket& SocketActive);

void socket_close(CActiveSocket SocketActive);

vector <int32_t> get_unloaded_num(vector<int32_t> segment_status);

void upload_traj(CActiveSocket& SocketActive, vector<vector<int32_t>> nodes);

vector <int32_t> download_traject (CActiveSocket &SocketActive, int32_t points_cnt);

vector<vector<int32_t>> get_nodes(const string& Traject_file_name);

string hex_converting(int num);
//////////////////////////////////////////////////////////////


///         request functions
//////////////////////////////////////////////////////////////
namespace request
{
constexpr size_t MAX_PACKET{4096};
const int32_t NODES_PER_PACKET{200};          // Number of points for 1 package
const uint32_t PACKETS_WO_CONFIRM{1};        // How many packages could be sent without confirmation
const double CONFIRM_TIMEOUT_SEC{0.1};
const int32_t POINTS_PER_PACKET{500};
string hex_converting(int32_t num)
{
    stringstream mystream;
    mystream << hex << num;
    if(num < 0)
    {
        return mystream.str().substr(4);
    }
    else
    {
        return mystream.str();
    }
}

void ShowError(CSimpleSocket ss, string s)
{

    std::cout << " " << s << " : " << " = " << ss.DescribeError() << std::endl;
    std::cout << " IsSocketValid() = " << ss.IsSocketValid() << std::endl << std::endl;
} //ShowError


void get_API_version (CActiveSocket& SocketActive){ //This function send API version request and shows response
    uint8 buf[MAX_PACKET] ;
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x0C);
    buf[3] = uint8(0x00);

    cout << "GET API VERSION";
    cout << "SocketActive.Send = " << SocketActive.Send(buf, 4) << endl;
    ShowError(SocketActive, "SocketActive.Send");


    cout << "listening..." << endl << endl;
    cout << "SocketActive.Receive = " << hex <<SocketActive.Receive(MAX_PACKET, buf) << endl;
    ShowError(SocketActive, "SocketActive.Receive");

/// @return number of bytes actually received.
/// @return of zero means the connection has been shutdown on the other side.
/// @return of -1 means that an error has occurred.

    cout << "Bytes Received : " ;
    for(int32_t i=0; i<SocketActive.GetBytesReceived(); i++)
        {
            //cout << " buf[" << ii << "] = " << buf[ii] << " " << endl;
            cout << hex << buf[i];
        } //for
        cout << endl << endl;
}// get_API_version



void get_sw_revision(CActiveSocket& SocketActive){ //This function send revision version request and shows response
    uint8 buf[MAX_PACKET] ;
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x0D);
    buf[3] = uint8(0x00);

    cout << "GET SW REVISION";
    cout << "SocketActive.Send = " << SocketActive.Send(buf, 4) << endl;
    ShowError(SocketActive, "SocketActive.Send");


    cout << "listening..." << endl << endl;
    cout << "SocketActive.Receive = " << hex <<SocketActive.Receive(MAX_PACKET, buf) << endl;
    ShowError(SocketActive, "SocketActive.Receive");

/// @return number of bytes actually received.
/// @return of zero means the connection has been shutdown on the other side.
/// @return of -1 means that an error has occurred.

    cout << "Bytes Received : " ;
    for(int32_t ii=0; ii<SocketActive.GetBytesReceived(); ii++)
        {
            //cout << " buf[" << ii << "] = " << buf[ii] << " " << endl;
            cout << hex << buf[ii];
        } //for
        cout << endl << endl;
}//get_sw_revision

void get_gru_state(CActiveSocket& SocketActive){ //This function send request of state gradient amplifier and shows response
    uint8 buf[MAX_PACKET] ;
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x05);
    buf[3] = uint8(0x00);

    cout << "GET GRU STATE";
    cout << "SocketActive.Send = " << SocketActive.Send(buf, 4) << endl;
    ShowError(SocketActive, "SocketActive.Send");


    cout << "listening..." << endl << endl;
    cout << "SocketActive.Receive = " << hex <<SocketActive.Receive(MAX_PACKET, buf) << endl;
    ShowError(SocketActive, "SocketActive.Receive");

/// @return number of bytes actually received.
/// @return of zero means the connection has been shutdown on the other side.
/// @return of -1 means that an error has occurred.

    cout << "Bytes Received : " ;
    for(int32_t ii=0; ii<SocketActive.GetBytesReceived(); ii++)
        {
            //cout << " buf[" << ii << "] = " << buf[ii] << " " << endl;
            cout << hex << buf[ii];
        } //for
        cout << endl << endl;

}//get_gru_state



void socket_close(CActiveSocket SocketActive){ // This function closes socket
std::cout << "SocketActive.Close() = " << SocketActive.Close() << std::endl;
    ShowError(SocketActive, "SocketActive.Close");
    cout << "closed" << endl;
}

vector<vector<int32_t>> get_nodes(const string& Traject_file_name) {
    vector<vector<int32_t>> nodes;
     ifstream myfile(Traject_file_name);
    if (!myfile.is_open()) { // ��������, ������� �� ������ ����
        cerr << "Unable to open file\n"; // ����� �� ��������� � �������
    }
    int32_t num1, num2;
    while(myfile >> num1 >> num2) {
        nodes.push_back({num1, num2});
    }

    return nodes;
}//get nodes

vector <int32_t> get_unloaded_num(vector<int32_t> segment_status){
    vector <int32_t> res;
    for (uint32_t i = 0; i< segment_status.size();i++){
        if (segment_status[i] != 0){
            res.push_back(i);
        }
    }
    return res;

}//get_unloaded_num

//function to upload a segment of traject
void upload_segment(CActiveSocket &SocketActive, int32_t seg_num, bool need_confirm, vector<vector<int32_t>> nodes)
{
    int32_t counter=0;
    uint8 buf[MAX_PACKET]{0};
    string str1 = hex_converting(seg_num);
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x07);
    buf[3] = uint8(0x00);
    counter+=4;

    //data of segment traject
    need_confirm = true;
if(need_confirm)
    {

    buf[4] = uint8(stoi(str1, nullptr, 16));
    buf[5] = uint8(0x80);
    counter+=2;

    }
    else{
        buf[4] = uint8(stoi(str1, nullptr, 16));
        counter++;
    }


    int32_t NULL32=0;
    int32_t NODES_SIZE = nodes.size();
    int32_t first_node_idx = max( seg_num * NODES_PER_PACKET - seg_num, NULL32 );
    int32_t last_node_idx = min( first_node_idx + NODES_PER_PACKET, NODES_SIZE) - 1;
    int32_t nodes_in_this_packet = last_node_idx - first_node_idx + 1;

    string str2 = hex_converting(nodes_in_this_packet);
    buf[counter++] = uint8(stoi(str2, nullptr, 16));
    buf[counter++] = uint8(0x00);

    for(int i = first_node_idx; i <last_node_idx+1; i++)
    {

        string hexString1 = hex_converting(nodes[i][0]);
        uint32_t tempcounter = counter;
        string temp1, temp2;

        for(int j = hexString1.length()-1; j > 0; j = j - 2)
        {
            buf[tempcounter++] = uint8(stoi(hexString1.substr(j-1,2), nullptr, 16) );
        }
        if(hexString1.length() % 2 != 0)
        {
            temp1 = hexString1[0];
            hexString1.erase(0);
            buf[tempcounter++] = uint8(stoi(temp1, nullptr, 16) );
        }

        counter+=4;

        uint32_t tempcounter2 = counter;

        string hexString2 = hex_converting(nodes[i][1]);

        for(int j = hexString2.length()-1; j > 0; j -= 2)
        {
            buf[tempcounter2++] = uint8(stoi(hexString2.substr(j-1,2), nullptr, 16) );
        }
        if(hexString2.length() % 2 != 0)
        {
            temp2 = hexString2[0];
            hexString1.erase(0);
            buf[tempcounter2++] = uint8(stoi(temp2, nullptr, 16) );
        }

        counter+=2;
    }


    cout << "SocketActive.Send = " << SocketActive.Send(buf, counter) << endl;
    ShowError(SocketActive, "SocketActive.Send");

}

void switch_func (string hexString, int32_t& counter, uint8 (&buf)[MAX_PACKET]){
    int tempcounter = counter;
    string temp1;
    for(int j = hexString.length()-1; j > 0; j=j-2)
        {
            buf[tempcounter++] = uint8(stoi(hexString.substr(j-1,2), nullptr, 16) );
        }
        if(hexString.length() % 2 != 0)
        {
            temp1 = hexString[0];
            hexString.erase(0);
            buf[tempcounter++] = uint8(stoi(temp1, nullptr, 16) );
        }
}

void upload_traj(CActiveSocket& SocketActive, vector<vector<int32_t>> nodes){
    //This function send trajectory to gradient amplifier
    int32_t counter = 0;
    uint8 buf[MAX_PACKET]{0} ;
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x06);
    buf[3] = uint8(0x00);
    counter+=4;

    string hexString = hex_converting(nodes.size());
    string hexString2 = hex_converting(nodes[nodes.size()-1][0]);

    switch_func(hexString2, counter, buf);
    counter+=4;
    switch_func(hexString, counter, buf);
    counter+=4;

    cout << "UPLOADING TRAJECTORY";
    cout << "SocketActive.Send = " << SocketActive.Send(buf, 12) << endl;
    ShowError(SocketActive, "SocketActive.Send");

    int32_t nodes_cnt = nodes.size();
    int32_t segments_cnt = nodes_cnt / NODES_PER_PACKET; // segments_count

    if (nodes_cnt % NODES_PER_PACKET != 0)
        segments_cnt += 1;

    vector<int32_t> segment_status(segments_cnt); //���������� �� ���������
    segment_status.assign(segments_cnt, -2);
    int32_t left_wo_confirm{PACKETS_WO_CONFIRM};

    //bool confirm_timeout_detected = false;
    //string prev_debug_info = " ";

    vector <int32_t> uploaded_nums;
    uploaded_nums = get_unloaded_num(segment_status);
    int32_t counter_uploaded_nums = uploaded_nums.size()-1;

    while (!uploaded_nums.empty()){

        int32_t seg_num;
        seg_num = uploaded_nums.front();

        if (segment_status[seg_num] != -2){
            cout << "Repeating upload segment" << seg_num << "with status" <<segment_status[seg_num] <<endl;
        }//if

        bool need_confirm = false;    //by default

        if (left_wo_confirm == 0)
            {
            left_wo_confirm = PACKETS_WO_CONFIRM;
            need_confirm = true;
            }
        else left_wo_confirm -= 1;    //for next iteration

        if (seg_num == counter_uploaded_nums)
            need_confirm = true;

        upload_segment(SocketActive, seg_num, need_confirm, nodes);

        cout << "listening..." << endl << endl;
        cout << "SocketActive.Receive = " << hex <<SocketActive.Receive(MAX_PACKET, buf) << endl;
        ShowError(SocketActive, "SocketActive.Receive");

        if(int32_t(buf[4]) == seg_num + 1)
        {
            uploaded_nums.erase(uploaded_nums.begin());
            cout<<"Uploaded: " << seg_num + 1 << endl;
            cout<< endl;
        }
        else
        {
            cerr<<"Fatal error 404"<<endl;
        }

    }//while


}//upload_traj

void download_traject (CActiveSocket &SocketActive, int32_t points_cnt)
{
    int32_t counter=0;
    vector<int32_t> points;
    uint8 buf[MAX_PACKET]{0} ;
    buf[0] = uint8(0xAA);
    buf[1] = uint8(0xAA);
    buf[2] = uint8(0x09);
    buf[3] = uint8(0x00);
    buf[4] = uint8(0x00);
    buf[5] = uint8(0x00);
    buf[6] = uint8(0x00);
    buf[7] = uint8(0x00);
    counter+=8;
    switch_func(hex_converting(points_cnt), counter, buf);
    cout << "DOWNLOADING TRAJECTORY";
    cout << "SocketActive.Send = " << SocketActive.Send(buf, 12) << endl;
    ShowError(SocketActive, "SocketActive.Send");
    uint32_t expected_packets_cnt = points_cnt / POINTS_PER_PACKET;
    if (points_cnt % POINTS_PER_PACKET != 0)
    {
        expected_packets_cnt += 1;
    }
    vector<int32_t> downloaded_segments;
    downloaded_segments.assign(expected_packets_cnt, 0);
    points.assign(points_cnt, 0);

    int32_t for_cnt=0;
    int32_t skiped_nums;


    FILE *file;
    char filename[] = "text.txt";
    file = fopen(filename, "a");
    if (file == NULL)
    {
        printf("Error\n", filename);

    }

    int null_counter=0;
    while(!downloaded_segments.empty())
    {
        skiped_nums = for_cnt == 0 ? 10 : 12;
        for_cnt++;
        downloaded_segments.pop_back();
        cout << "listening..." << endl << endl;
        cout << "SocketActive.Receive = " << hex <<SocketActive.Receive(MAX_PACKET, buf) << endl;
        ShowError(SocketActive, "SocketActive.Receive");


        cout << "Bytes Received : " ;

        for(int32_t ii=10; ii<SocketActive.GetBytesReceived(); ii+=2)
            {
                 fprintf(file, "%d\n", int16_t(buf[ii+1] << 8 | buf[ii])) ;
            } //for
        cout << endl << endl;
    }




}
} //namespace request
//////////////////////////////////////////////////////////////


int main()
{
    SetConsoleCP(866);
    SetConsoleOutputCP(866);
    int32_t points_cnt;
    CActiveSocket SocketActive( CSimpleSocket::CSocketType::SocketTypeUdp) ;
    cout << "starting" << endl;
    // Initialize our socket object
    cout << "SocketActive.Initialize = " << SocketActive.Initialize() << endl;
    request::ShowError(SocketActive, "SocketActive.Initialize");
    cout << "SocketActive.Open = " << SocketActive.Open("192.168.100.3", 5002) << endl;
    request::ShowError(SocketActive, "SocketActive.Open");


    request::get_API_version(SocketActive); // request API version
    request::get_sw_revision(SocketActive); // request GRU software version
    request::get_gru_state(SocketActive);  // request GRU state
    string Traject_file_name, answer;
    cout << "Do you want to use default traject? [y/n]";
    cin >> answer;

    if (answer == "n") {
        cout << "Input file name (with extension):";
        cin >> Traject_file_name;
    }
    else Traject_file_name = "traject.txt";

    auto nodes = request::get_nodes(Traject_file_name); //filling vector nodes from data file
    int32_t TEMP_SIZE_NODE = nodes.size(); //using for download
    points_cnt = nodes[TEMP_SIZE_NODE-1][0]; //using for download

    for(auto &node : nodes)
    {
        cout<<node[0]<<"\t"<<node[1]<<endl; //vector nodes, output
    }
    cout << "Filled array:"<<endl;

    request::upload_traj(SocketActive, nodes); //uploading trajectory
    request::download_traject (SocketActive, points_cnt); // downloading trajectory
    request::socket_close(SocketActive);   // socket closing

    return 0;
}//main
