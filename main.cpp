#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <windows.h>
#include <dirent.h>

using namespace std;

struct st_seq
{
    st_seq()
    {
        antiparallel=false;
        longest_streak=0;
    }

    string id;
    string name;
    bool antiparallel;
    string seq;
    string seq_template;
    string group_name;
    int longest_streak;
};

string make_antiparallel(string seq);

int main()
{
    cout<<"Sequence Data Processing\n\n";
    cout<<"Reads the seq_list.xls along with the .fasta files in the local folder\n";
    cout<<"Example: Sequence ID EH01, Sequence name f_NAME1\n";
    cout<<"         f_ or r_ state reaction direction\n";
    cout<<"         End value (0-9) states sequences to be aligned\n";
    cout<<"A template sequence should be named template_NAME.txt\n";
    cout<<"Requires MS Excel and ClustalW to run.\n\n";
    string line;
    vector<st_seq> vec_seq;
    string file_list_name;

    //read all sequences
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (".")) != NULL)
    {
        //print all the files and directories within directory
        while ((ent = readdir (dir)) != NULL)
        {
            string filename=ent->d_name;
            //cout<<filename<<endl;
            //xls check
            if(filename[(int)filename.size()-4]=='.'&&filename[(int)filename.size()-3]=='x'&&
               filename[(int)filename.size()-2]=='l'&&filename[(int)filename.size()-1]=='s')
            {
                file_list_name=filename;
            }

            if((int)filename.size()<7) continue;
            if(filename[(int)filename.size()-6]=='.'&&filename[(int)filename.size()-5]=='f'&&
               filename[(int)filename.size()-4]=='a'&&filename[(int)filename.size()-3]=='s'&&
               filename[(int)filename.size()-2]=='t'&&filename[(int)filename.size()-1]=='a')
            {
                vec_seq.push_back(st_seq());
                vec_seq.back().id=string(filename,0,4);

                //get seq data
                filename="";
                filename.append(ent->d_name);
                ifstream seq_file(filename.c_str());
                if(seq_file==0)
                {
                    cout<<"ERROR: Bad read of .fasta file\n";
                    return 3;
                }
                while(getline(seq_file,line))
                {
                    if(line[0]!='>')
                    {
                        vec_seq.back().seq.append(line);
                    }
                }
                seq_file.close();
            }
        }
        closedir (dir);
    }
    else
    {
        cout<<"ERROR: Could not open folder info\n";
        return 2;
    }

    //ask for file list
    bool have_xls_name=false;
    if(!file_list_name.empty())
    {
        cout<<"\nAre the sequences listed in "<<file_list_name<<"? (y/n): ";
        getline(cin,line);
        if(line[0]=='n'||line[0]=='N')
        {
            //ask for new name
            have_xls_name=false;
            cout<<endl;
        }
        else//keep name
        {
            have_xls_name=true;
            cout<<endl;
        }
    }
    if(!have_xls_name)
    {
        cout<<"Enter name of the .xls file list: ";
        getline(cin,line);
        file_list_name=line;
        if((int)file_list_name.size()<4)
        {
            cout<<"ERROR: Bad .xls name\n";
            return 9;
        }
    }

    //convert excel to csv
    string file_list_name_old=file_list_name;
    file_list_name[(int)file_list_name.size()-3]='c';
    file_list_name[(int)file_list_name.size()-2]='s';
    file_list_name[(int)file_list_name.size()-1]='v';
    //create script
    ofstream script_file("ExcelToCsv.vbs");
    if(script_file==0)
    {
        cout<<"ERROR: Could not create file\n";
        return 10;
    }
    script_file<<"if WScript.Arguments.Count < 2 Then\n";
    script_file<<"WScript.Echo \"Please specify the source and the destination files. Usage: ExcelToCsv <xls/xlsx source file> <csv destination file>\"\n";
    script_file<<"Wscript.Quit\n";
    script_file<<"End If\n";
    script_file<<"csv_format = 6\n";
    script_file<<"Set objFSO = CreateObject(\"Scripting.FileSystemObject\")\n";
    script_file<<"src_file = objFSO.GetAbsolutePathName(Wscript.Arguments.Item(0))\n";
    script_file<<"dest_file = objFSO.GetAbsolutePathName(WScript.Arguments.Item(1))\n";
    script_file<<"Dim oExcel\n";
    script_file<<"Set oExcel = CreateObject(\"Excel.Application\")\n";
    script_file<<"Dim oBook\n";
    script_file<<"Set oBook = oExcel.Workbooks.Open(src_file)\n";
    script_file<<"oBook.SaveAs dest_file, csv_format\n";
    script_file<<"oBook.Close False\n";
    script_file<<"oExcel.Quit\n";
    script_file.close();
    string command_to_run="ExcelToCsv.vbs ";
    command_to_run.append(file_list_name_old);
    command_to_run.append(" ");
    command_to_run.append(file_list_name);
    system(command_to_run.c_str());
    remove("ExcelToCsv.vbs");

    //read input files (excel)
    ifstream csv_file(file_list_name.c_str());
    if(csv_file==0)
    {
        cout<<"ERROR: Could not find file\n";
        return 1;
    }
    getline(csv_file,line);//skip header
    while(getline(csv_file,line))
    {
        bool have_id=false;
        string file_id="";
        string file_name="";
        for(int i=1;i<(int)line.size();i++)
        {
            if(!have_id)
            {
                if(line[i]==',')
                {
                    have_id=true;
                    //cout<<file_id<<endl;
                    continue;
                }
                file_id.append(1,line[i]);
            }
            else
            {
                if(line[i]==',')
                {
                    //cout<<file_name<<endl;
                    break;
                }
                file_name.append(1,line[i]);
            }
        }
        //cout<<file_id<<" - "<<file_name<<endl;
        if(file_id.empty()||file_name.empty())
        {
            //empty line, ignore
            continue;

            //cout<<"ERROR: Bad file names in excel list\n";
            //return 4;
        }

        //find id in seq list
        bool seq_found=false;
        for(int i=0;i<(int)vec_seq.size();i++)
        {
            if(vec_seq[i].id==file_id)
            {
                seq_found=true;
                vec_seq[i].name=file_name;
                string name_wo_num=string(file_name,0,(int)file_name.size()-1);
                vec_seq[i].group_name=name_wo_num;
                //cout<<name_wo_num<<endl;
                if(file_name[0]=='f'||file_name[0]=='F')
                {
                    vec_seq[i].antiparallel=false;
                }
                else if(file_name[0]=='r'||file_name[0]=='R')
                {
                    vec_seq[i].antiparallel=true;
                }
                else
                {
                    cout<<"WARNING: File names should start with \"f\" or \"r\" stating forward or reverse reaction\n";
                }
                break;
            }
        }
        if(!seq_found)
        {
            cout<<"ERROR: .fasta file found but not in the excel list\n";
            return 5;
        }
    }
    csv_file.close();

    //remove csv
    remove(file_list_name.c_str());

    //make anti-parallel
    for(int i=0;i<(int)vec_seq.size();i++)
    {
        if(vec_seq[i].antiparallel)
        {
            vec_seq[i].seq=make_antiparallel(vec_seq[i].seq);
        }
    }

    //output all seq
    ofstream out_all_seq("sum.txt");
    if(out_all_seq==0)
    {
        cout<<"ERROR: Could not  create output file\n";
        return 6;
    }
    for(int i=0;i<(int)vec_seq.size();i++)
    {
        out_all_seq<<'>'<<vec_seq[i].name<<endl;
        out_all_seq<<vec_seq[i].seq<<endl<<endl;
    }
    out_all_seq.close();

    //split seq into groups based on similar reactions
    vector<string> vec_groups;
    vector<string> vec_templates;
    for(int i=0;i<(int)vec_seq.size();i++)
    {
        //check for template
        string template_name=string(vec_seq[i].group_name,2);

        //find template file
        string file_name="template_";
        file_name.append(template_name);
        file_name.append(".txt");
        //cout<<"find: "<<file_name<<endl;
        ifstream template_file(file_name.c_str());
        if(template_file==0)
        {
            ;//no template for this group
        }
        else
        {
            //read sequence
            string seq="";
            while(getline(template_file,line))
            {
                if(line[0]!='>')
                {
                    seq.append(line);
                }
            }

            vec_seq[i].seq_template=seq;
        }
        template_file.close();

        //check if group exists
        bool group_found=false;
        for(int j=0;j<(int)vec_groups.size();j++)
        {
            if(vec_groups[j]==vec_seq[i].group_name)
            {
                group_found=true;
                break;
            }
        }
        if(!group_found)
        {
            //create new
            vec_groups.push_back(vec_seq[i].group_name);
        }
    }

    //make clustal input files
    for(int i=0;i<(int)vec_groups.size();i++)
    {
        string filename=vec_groups[i];
        filename.append(".txt");
        ofstream out_file(filename.c_str());
        if(out_file==0)
        {
            cout<<"ERROR: Could not create file\n";
            return 7;
        }
        bool template_seq_printed=false;
        for(int j=0;j<(int)vec_seq.size();j++)
        {
            if(vec_seq[j].group_name==vec_groups[i])
            {
                if(!template_seq_printed && !vec_seq[j].seq_template.empty())
                {
                    template_seq_printed=true;
                    string template_name=string(vec_seq[j].group_name,2);
                    out_file<<'>'<<template_name<<"_template"<<endl;
                    out_file<<vec_seq[j].seq_template<<endl<<endl;
                }

                //add seq to file
                out_file<<'>'<<vec_seq[j].name<<endl;
                out_file<<vec_seq[j].seq<<endl<<endl;
            }
        }
        out_file.close();
    }

    //run clustal
    for(int i=0;i<(int)vec_groups.size();i++)
    {
        string input_file=vec_groups[i];
        input_file.append(".txt");

        string command_to_run="clustalw2 -TYPE=DNA -INFILE=";
        command_to_run.append(input_file);

        system(command_to_run.c_str());
        input_file[(int)input_file.size()-3]='d';
        input_file[(int)input_file.size()-2]='n';
        input_file[(int)input_file.size()-1]='d';
        remove(input_file.c_str());
    }


    //alignment analysis
    //...determine if ok or not, consider f and r reactions

    //align all seq to its templates only
    for(int i=0;i<(int)vec_groups.size();i++)
    {
        for(int j=0;j<(int)vec_seq.size();j++)
        {
            if(vec_groups[i]==vec_seq[j].group_name && !vec_seq[j].seq_template.empty())
            {
                string filename=vec_seq[j].name;
                filename.append(".txt");
                ofstream out_file(filename.c_str());
                if(out_file==0)
                {
                    cout<<"ERROR: Could not create file\n";
                    return 11;
                }

                //add template to file
                string template_name=string(vec_seq[j].group_name,2);
                out_file<<'>'<<template_name<<"_template"<<endl;
                out_file<<vec_seq[j].seq_template<<endl<<endl;

                //add seq to file
                out_file<<'>'<<vec_seq[j].name<<endl;
                out_file<<vec_seq[j].seq<<endl<<endl;

                out_file.close();

                //align
                string command_to_run="clustalw2 -TYPE=DNA -INFILE=";
                command_to_run.append(filename);
                system(command_to_run.c_str());
                filename[(int)filename.size()-3]='d';
                filename[(int)filename.size()-2]='n';
                filename[(int)filename.size()-1]='d';
                remove(filename.c_str());

                //find stars
                filename[(int)filename.size()-3]='a';
                filename[(int)filename.size()-2]='l';
                filename[(int)filename.size()-1]='n';
                ifstream align_file(filename.c_str());
                if(align_file==0)
                {
                    cout<<"ERROR: Could not find alignment file\n";
                    return 14;
                }
                int curr_streak=0;
                int longest_streak=0;
                int start_letter=int(template_name.size())+9+6; //assuming that the template has the longest name and a spacer of 6 spaces (9 _template)
                while(getline(align_file,line))
                {
                    if((int)line.size()<=start_letter) continue;//gap line
                    if(line[start_letter]!='*'&&line[start_letter]!=' ') continue;//sequence line

                    //bool before_seq_region=true;
                    for(int let=start_letter;let<(int)line.size();let++)
                    {
                        if(line[let]=='*')
                        {
                            //before_seq_region=false;
                            curr_streak++;

                            //cout<<line[let];
                        }
                        else if(line[let]==' ')
                        {
                            if(curr_streak>longest_streak) longest_streak=curr_streak;
                            curr_streak=0;

                            //cout<<'x';
                        }
                    }
                }
                cout<<endl;
                align_file.close();
                //store score
                vec_seq[j].longest_streak=longest_streak;
            }
        }
    }

    //print best score within group
    ofstream res_file("analysis.txt");
    if(res_file==0)
    {
        cout<<"ERROR: Could not create file\n";
        return 13;
    }
    res_file<<"Sequence analysis results:\n\nFile with longest matching region within group:\n";
    for(int i=0;i<(int)vec_groups.size();i++)
    {
        string group_winner;
        int highest_score=0;
        for(int j=0;j<(int)vec_seq.size();j++)
        {
            if(vec_groups[i]==vec_seq[j].group_name)
            {
                if(vec_seq[j].longest_streak>highest_score)
                {
                    group_winner=vec_seq[j].name;
                    highest_score=vec_seq[j].longest_streak;
                }
            }
        }
        res_file<<vec_groups[i]<<": "<<group_winner<<": "<<highest_score<<endl;
    }
    res_file<<endl;
    res_file.close();

    cout<<"\nAlignment complete\n\n";

    return 0;
}

string make_antiparallel(string seq)
{
    if(seq.empty())
    {
        cout<<"WARNING: Call to make an empty sequence antiparallel\n";
        return "";
    }
    string antipar;
    for(int i=(int)seq.size()-1;i>=0;i--)
    {
        switch(seq[i])
        {
            case 'A': antipar.append(1,'T'); break;
            case 'T': antipar.append(1,'A'); break;
            case 'G': antipar.append(1,'C'); break;
            case 'C': antipar.append(1,'G'); break;
            case 'a': antipar.append(1,'t'); break;
            case 't': antipar.append(1,'a'); break;
            case 'g': antipar.append(1,'c'); break;
            case 'c': antipar.append(1,'g'); break;
            default : antipar.append(1,'X'); break;
        }
    }

    return antipar;
}
