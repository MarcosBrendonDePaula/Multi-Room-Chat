#include <vector>
#include <map>
#include <exception>
#include "EasyLib/EasyMultServer.h"
#include "EasyLib/Events.h"

Events *eventos;
EasyMultServer *servidor;

map<int,vector<Nsock*>> salas;
map<Nsock*,int> UserSala;
map<Nsock*,bool> Dc;
map<Nsock*,string> nicknames;
const vector<string> split(const string& s, const char& c)
{
	string buff{""};
	vector<string> v;
	for(auto n:s)
	{
		if(n != c) buff+=n; else
		if(n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if(buff != "") v.push_back(buff);	
	return v;
}


void msgSala(int sala_n,string msg,Nsock* user)
{
    vector<Nsock*> *sala = &salas[sala_n];
    cout<<sala_n<<":"<<sala->size()<<endl;
    for(int i=0;i<sala->size();i++)
    {
        if(!Dc[(*sala)[i]] && user!=(*sala)[i])
            if(nicknames[user]=="")
                (*sala)[i]->SendMsg(to_string(user->id)+":"+msg);
            else
                (*sala)[i]->SendMsg(nicknames[user]+":"+msg);
    }
}

void AnuncioSala(int sala_n,Nsock *user)
{
    vector<Nsock*> *sala = &salas[sala_n];

    for(int i=0;i<sala->size();i++)
    {
        if(!Dc[(*sala)[i]] && user!=(*sala)[i])
            if(nicknames[user]=="")
                (*sala)[i]->SendMsg("ID > "+to_string(user->id)+" Se juntou a sala");
            else
                (*sala)[i]->SendMsg(nicknames[user]+" Se juntou a sala");
    }
}
void DcFunction(void *arg){
    Nsock *Sock=(Nsock*)arg;
    cout<<"ID:"<<Sock->id<<" Desconectou"<<endl;
    Dc[Sock]=true;
}
void response(void *arg)
{
    Nsock *Conexao = (Nsock*)arg;
    string msg = Conexao->getInput();
    if(msg.find("sala,")==0)
    {
        vector<string> dados{split(msg,',')};
        int localAnterior = UserSala[Conexao];
        vector<Nsock*>::iterator i;
        bool achou = false;
        int deslocamento=0;
        for(i = salas[localAnterior].begin();i!=salas[localAnterior].end();i++)
        {
            if((*i) == Conexao){
                achou = true;
                cout<<"Encontrado"<<endl;
                break; 
            }
            deslocamento++;
        }
        if(!achou)
            return;
        UserSala[Conexao] = stoi(dados[1]);
        salas[localAnterior].erase(salas[localAnterior].begin()+deslocamento);
        AnuncioSala(UserSala[Conexao],Conexao);
        salas[UserSala[Conexao]].push_back(Conexao);
        
    }
    else if(msg.find("nick,")==0)
    {
        vector<string> dados{split(msg,',')};
        nicknames[Conexao] = dados[1];
    }
    else
    {
        msgSala(UserSala[Conexao],msg,Conexao);
    }
}

void conectado(void *arg)
{
    Nsock *Conexao = (Nsock*)arg;
    nicknames[Conexao] = "";
    UserSala[Conexao]=0;
    AnuncioSala(0,Conexao);
    salas[0].push_back(Conexao);
}

void PreloadSalas(int qt)
{
    for(int i=0;i<qt;i++)
    {
        vector<Nsock*> sala;
        salas[i] = sala;
    }
}

int main(){
    eventos = new Events();
    eventos->Paralel = true;
    servidor = new EasyMultServer(25565,eventos);
    PreloadSalas(45000);
    servidor->Start(&response,400,&conectado,&DcFunction);
    if(!servidor->isOpened())
    {
        return 1;
    }
    int x;
    cin>>x;
    return 0;
}