#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <iterator>
#include <list>
#include <vector>
#include <sstream>
#include <ctime>
#include <algorithm>

using namespace std;

int start_s = clock();

//FILE HANDLING VARIABLES
ifstream node_file, net_file, aux_file;
ofstream output_f;

//NET AND NODE MAP CONSTRUCTION VARIABLES
string nodeline, node_type, netline, net_nodeline, netname, current_net, curr_net, newnetname;
int instance = 0, weight = 0, part_count = 0, nodename = 0, count1 = 0, countn1 = 0, l = 0, cell = 0;
float initial_cutset = 0, cutset = 0, wptot = 0;
float wp1 = 0, wpn1 = 0, wp1_mincut = 0, wpn1_mincut = 0;
int current_node = 0, current_wgt = 0, current_part = 0, net_count1 = 0, net_countn1 = 0, gain = 0, cutset_netcard = 0;
int base_cell = 0, base_wgt = 0, base_part = 0, gain_old = 0, gain_new = 0, fg = 1;
int x = 0, p = 0, q = 0, gain_cell = 0, max_gain = 0, cum_gain = 0;
int check_cell = 0, check_part = 0, check_wgt = 0, nb_node = 0, node_gain = 0, count_p = 0, final_cutset = 0;
int max_gain_pos = 0;
int cum_gain_pos = 0;

//flags
bool STOP = true, found_baseC1 = false, found_baseCn1 = false, is_benc = false;

//vector used to seperate lines read from files
vector<string> sep_node, sep_node2;

//MAPS FOR NODES
map<int, vector<int> > node_map; //3 elements, 0 wgt, 1 part, 2 fix, 3 gain
map<int, vector<string> > node_net_map; // node to nets
map<string, vector<int> > net_node_map; //net to nodes
map<int, vector<int> > gain_bucket; // key: gain, values: nodes
map<string, vector<int> > net_card; //MAP FOR NETS (net sizes)

//defining vectors
vector<int> free_nodes;
vector<int> fixed_cell_list;

//Iterators used
typedef map<int, vector<int> >::reverse_iterator gain_buk_iter;
typedef map<int, vector<int> >::iterator iter;
typedef map<int, vector<int> >::iterator n_v;
typedef map<int, vector<int> >::iterator nodeiter;
typedef map<int, vector<string> >::iterator net_nodeiter;
typedef map<string, vector<int> >::iterator netcarditer;
typedef map<string, vector<int> >::iterator netcarditer1;
typedef map<string, vector<int> >::iterator netnodeiter;


//SPLIT FUNCTION (MOVE TO ANOTHER FILE)
vector<string> split(string str, char delimiter) {
	vector<string> internal;
	stringstream ss(str); // Turn the string into a stream.
	string tok;

	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}
	return internal;
}

void re_cal_DS()
{
	//erasing entire net card map
	net_card.clear();

	l = 0;
	cell = 0;
	count1 = 0;
	countn1 = 0;

	//reverting back the changes
	for (l = max_gain_pos; l < fixed_cell_list.size(); l++)
	{
		cell = fixed_cell_list[l];
		node_map[cell][1] = -1 * (node_map[cell][1]);
	}

	//unfix all fix nodes
	for (nodeiter r = node_map.begin(); r != node_map.end(); r++)
	{
		node_map[r->first][2] = 0;
	}

	//re building net card
	for (netnodeiter t = net_node_map.begin(); t != net_node_map.end(); t++)
	{
		count1 = 0;
		countn1 = 0;
		newnetname = t->first;
		for (int y = 0; y < t->second.size(); y++)
		{
			if (node_map[net_node_map[newnetname][y]][1] == 1)
			{
				count1 = count1 + 1;
			}
			if (node_map[net_node_map[newnetname][y]][1] == -1)
			{
				countn1 = countn1 + 1;
			}
		}
		net_card[newnetname].push_back(count1);
		net_card[newnetname].push_back(countn1);
	}

	//re building gain bucket
	for (int i = 0; i < free_nodes.size(); i++)
	{
		int current_node = free_nodes.at(i);
		gain = 0;
		for (int var = 0; var < node_net_map[current_node].size(); var++)
		{
			current_net = node_net_map[current_node].at(var);
			net_count1 = net_card[current_net].at(0);
			net_countn1 = net_card[current_net].at(1);
			current_wgt = node_map[current_node].at(0);
			current_part = node_map[current_node].at(1);
			if (current_part == 1)
			{
				if (net_count1 == 1)
				{
					gain = gain + 1;
				}
				else if (net_countn1 == 0)
				{
					gain = gain - 1;
				}
			}
			else
			{
				if (net_count1 == 0)
				{
					gain = gain - 1;
				}
				else if (net_countn1 == 1)
				{
					gain = gain + 1;
				}
			}
		}
		gain_bucket[gain].push_back(current_node);
		node_map[current_node][3] = gain;
	}

	//reinitializing fix bucket
	fixed_cell_list.clear();

	//Realizing the FM Algorithm
	//Variables
	gain_cell = 0;
	wp1 = wp1_mincut;
	wpn1 = wpn1_mincut;

}

void update_gain()
{
	//updating of gain of neighbours
	if (found_baseC1)
	{
		if (!gain_bucket.empty())
		{
			for (p = 0; p < node_net_map[base_cell].size(); p++)
			{
				curr_net = node_net_map[base_cell][p];
				if (net_card[curr_net][1] == 0)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						nb_node = net_node_map[curr_net][q];
						if (node_map[nb_node][2] == 0)
						{
							gain_old = node_map[nb_node][3];
							gain_new = gain_old + 1;
							node_map[nb_node][3] = node_map[nb_node][3] + 1;

							gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
							gain_bucket[gain_new].push_back(nb_node);

						}
					}
				}
				else if (net_card[curr_net][1] == 1)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						if (node_map[net_node_map[curr_net][q]][1] == -1)
						{
							nb_node = net_node_map[curr_net][q];
							if (node_map[nb_node][2] == 0)
							{
								gain_old = node_map[nb_node][3];
								gain_new = gain_old - 1;
								node_map[nb_node][3] = gain_new;

								gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
								gain_bucket[gain_new].push_back(nb_node);

							}
						}
					}
				}

				//Changing Net Cardinality
				net_card[curr_net][0] = net_card[curr_net][0] - 1;
				net_card[curr_net][1] = net_card[curr_net][1] + 1;

				if (net_card[curr_net][0] == 0)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						nb_node = net_node_map[curr_net][q];
						if (node_map[nb_node][2] == 0)
						{
							gain_old = node_map[nb_node][3];
							gain_new = gain_old - 1;
							node_map[nb_node][3] = node_map[nb_node][3] - 1;

							gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
							gain_bucket[gain_new].push_back(nb_node);

						}
					}
				}
				else if (net_card[curr_net][0] == 1)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						if (node_map[net_node_map[curr_net][q]][1] == 1)
						{
							nb_node = net_node_map[curr_net][q];
							if (node_map[nb_node][2] == 0)
							{
								gain_old = node_map[nb_node][3];
								gain_new = gain_old + 1;
								node_map[nb_node][3] = gain_new;

								gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
								gain_bucket[gain_new].push_back(nb_node);

							}
						}
					}
				}
			}
		}
	}

	if (found_baseCn1)
	{
		if (!gain_bucket.empty())
		{
			for (p = 0; p < node_net_map[base_cell].size(); p++)
			{
				curr_net = node_net_map[base_cell][p];
				if (net_card[curr_net][0] == 0)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						nb_node = net_node_map[curr_net][q];
						if (node_map[nb_node][2] == 0)  //checking if node is fixed
						{
							gain_old = node_map[nb_node][3];
							gain_new = gain_old + 1;
							node_map[nb_node][3] = node_map[nb_node][3] + 1;


							gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
							gain_bucket[gain_new].push_back(nb_node);

						}
					}
				}
				else if (net_card[curr_net][0] == 1)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						if (node_map[net_node_map[curr_net][q]][1] == 1)
						{
							nb_node = net_node_map[curr_net][q];
							if (node_map[nb_node][2] == 0)
							{
								gain_old = node_map[nb_node][3];
								gain_new = gain_old - 1;
								node_map[nb_node][3] = gain_new;
								gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
								gain_bucket[gain_new].push_back(nb_node);
							}
						}
					}
				}

				//Changing Net Cardinality
				net_card[curr_net][1] = net_card[curr_net][1] - 1;
				net_card[curr_net][0] = net_card[curr_net][0] + 1;

				if (net_card[curr_net][1] == 0)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						nb_node = net_node_map[curr_net][q];
						if (node_map[nb_node][2] == 0)  //checking if node is fixed
						{
							gain_old = node_map[nb_node][3];
							gain_new = gain_old - 1;
							node_map[nb_node][3] = node_map[nb_node][3] - 1;

							gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
							gain_bucket[gain_new].push_back(nb_node);

						}
					}

				}
				else if (net_card[curr_net][1] == 1)
				{
					for (q = 0; q < net_node_map[curr_net].size(); q++)
					{
						if (node_map[net_node_map[curr_net][q]][1] == -1)
						{
							nb_node = net_node_map[curr_net][q];
							if (node_map[nb_node][2] == 0)
							{
								gain_old = node_map[nb_node][3];
								gain_new = gain_old + 1;
								node_map[nb_node][3] = gain_new;

								gain_bucket[gain_old].erase(remove(gain_bucket[gain_old].begin(), gain_bucket[gain_old].end(), nb_node), gain_bucket[gain_old].end());
								gain_bucket[gain_new].push_back(nb_node);
							}
						}
					}
				}
			}
		}
	}
}

void delete_base_and_empty_keys()
{
	//Deleting empty keys:
	if (!gain_bucket.empty())
	{
		while (gain_bucket[gain_bucket.begin()->first].empty())
		{
			gain_bucket.erase(gain_bucket.begin()->first);
		}
	}

	//removing fixed elements from gain bucket
	if (found_baseC1 || found_baseCn1)
	{
		if (!gain_bucket.empty())
		{
			if (gain_bucket[node_map[base_cell][3]].size() == 1)
			{
				gain_bucket.erase(node_map[base_cell][3]);
				if (!gain_bucket.empty())
				{

					while (gain_bucket[gain_bucket.rbegin()->first].empty())
					{
						gain_bucket.erase(gain_bucket.rbegin()->first);
					}
				}
			}
			else
			{
				gain_bucket[node_map[base_cell][3]].erase(remove(gain_bucket[node_map[base_cell][3]].begin(), gain_bucket[node_map[base_cell][3]].end(), base_cell), gain_bucket[node_map[base_cell][3]].end());
			}
		}
	}
}
int cutset_cal(int cutset)
{
	//Realizing the FM Algorithm
	//Variables
	x = 0; p = 0; q = 0; gain_cell = 0; max_gain = 0; cum_gain = 0;
	STOP = true; found_baseC1 = false; found_baseCn1 = false;
	check_cell = 0; check_part = 0; check_wgt = 0; nb_node = 0; node_gain = 0; count_p = 0; final_cutset = 0;
	max_gain_pos = 0;
	cum_gain_pos = 0;

	////////////////////////////////////////////////////////////////////////////START OF THE PASS//////////////////////////////////////////////////////////////////////////////////////
	while (STOP)
	{
		//Iterating the gain bucket
		for (gain_buk_iter b = gain_bucket.rbegin(); b != gain_bucket.rend(); b++) //Scanning key in DECENDING ORDER.
		{
			//Initializing checks for finding cells.
			found_baseC1 = false;
			found_baseCn1 = false;
			for (x = 0; x < b->second.size(); x++) //Scanning all cells in a particular key.
			{
				check_cell = b->second[x];
				gain_cell = b->first;
				if (node_map[check_cell][2] == 0) //Checking if Cell is fixed or not.
				{
					check_part = node_map[check_cell][1];
					check_wgt = node_map[check_cell][0];
					if (check_part == 1) //Checking in which partition does the cell lie.
					{
						if ((((wp1 - check_wgt) / (wp1 + wpn1)) >= 0.4)) //Checking if the cell satisfies the balance criterion.
						{
							//Qualifying the cell as the base cell.
							base_cell = check_cell;
							base_part = check_part;
							base_wgt = check_wgt;
							//MOVING THE CELL//
							//Changing the cell partition
							node_map[base_cell][1] = -1; //* (base_part);
							node_map[base_cell][2] = 1; //Fixing the cell
							wp1 = wp1 - base_wgt;
							wpn1 = wpn1 + base_wgt;

							fixed_cell_list.push_back(base_cell);

							cum_gain = cum_gain + gain_cell;
							cum_gain_pos = cum_gain_pos + 1;

							if (max_gain < cum_gain)
							{
								max_gain = cum_gain;
								max_gain_pos = cum_gain_pos;
								wp1_mincut = wp1;
								wpn1_mincut = wpn1;
							}

							found_baseC1 = true;
							break;
						}
					}
					else
					{
						if ((((wpn1 - check_wgt) / (wp1 + wpn1)) >= 0.4)) //Checking if the cell satisfies the balance criterion.
						{
							base_cell = check_cell;
							base_part = check_part;
							base_wgt = check_wgt;
							//MOVING THE CELL
							node_map[base_cell][1] = 1; // *(base_part); //change Part
							node_map[base_cell][2] = 1;
							wp1 = wp1 + base_wgt;
							wpn1 = wpn1 - base_wgt;

							fixed_cell_list.push_back(base_cell);

							cum_gain = cum_gain + gain_cell;
							cum_gain_pos = cum_gain_pos + 1;

							if (max_gain < cum_gain)
							{
								max_gain = cum_gain;
								max_gain_pos = cum_gain_pos;
								wp1_mincut = wp1;
								wpn1_mincut = wpn1;
							}

							found_baseCn1 = true;
							break;
						}
					}
				}
			}

			if (found_baseC1 || found_baseCn1)
			{
				break;
			}
		}

		//deleting the base cell from the gain bucket and also the empty keys created from updating gains of neighbours
		delete_base_and_empty_keys();

		//calling update gain function
		update_gain();

		//stop condition for the pass
		if (!gain_bucket.empty())
		{
			if ((gain_bucket[gain_bucket.begin()->first][(gain_bucket[gain_bucket.begin()->first].size() - 1)] == check_cell) && (check_cell != base_cell))
			{
				STOP = false;
			}
		}
		else
		{
			STOP = false;
		}
		//count_p = count_p + 1;
	}
	//final cutset
	cutset = cutset - max_gain;
	///////////////////////////////////////////////////////////////////////////////end of the pass /////////////////////////////////////////////////////////////////////////////

	//re constructing data structure
	re_cal_DS();
	return cutset;
}

////////////////////////////////////////////main starts here/////////////////////////////////////////
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "missing argument: " << endl;
		cout << "Please provide files in correct format." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string aux_f, auxline;
		vector<string> aux_val;
		aux_f = argv[1];
		const char* aux_fi = argv[1];
		string node_f;
		string net_f;
		bool is_node = false, is_net = false;
		if (aux_f == "design.aux");
		{
			aux_file.open(aux_fi);
			while (getline(aux_file, auxline))
			{
				aux_val = split(auxline, ' ');
				for (int au = 0; au < aux_val.size(); au++)
				{
					if (aux_val[au] == "design.nodes")
					{
						node_f = aux_val[au];
						is_node = true;
					}
					if (aux_val[au] == "design.nets")
					{
						net_f = aux_val[au];
						is_net = true;
					}
					if (is_node && is_net)
					{
						break;
					}
				}
			}
		}
		if (is_node && is_net)
		{
			//Fetching Data from Nodes file;
			node_file.open(node_f.c_str());
			while (getline(node_file, nodeline))
			{
				sep_node = split(nodeline, ' ');
				node_type = sep_node[1];
				sep_node2 = split(sep_node[0], '_');
				istringstream(sep_node2[1]) >> instance;
				free_nodes.push_back(instance);

				if (node_type == "FDRE") weight = 5;
				else if (node_type == "LUT6") weight = 7;
				else if (node_type == "LUT5") weight = 6;
				else if (node_type == "LUT4") weight = 5;
				else if (node_type == "LUT3") weight = 4;
				else if (node_type == "LUT2") weight = 3;
				else if (node_type == "LUT1") weight = 2;
				else if (node_type == "CARRY8") weight = 34;
				else if (node_type == "DSP48E") weight = 429;
				else if (node_type == "RAMB36E2") weight = 379;
				else if (node_type == "BUFGCE") weight = 3;
				else if (node_type == "IBUF") weight = 2;
				else if (node_type == "OBUF") weight = 2;

				if ((part_count % 2) == 0) {
					node_map[instance].push_back(weight);
					node_map[instance].push_back(1);
					wp1 = wp1 + weight;
				}
				else if ((part_count % 2) != 0) {
					node_map[instance].push_back(weight);
					node_map[instance].push_back(-1);
					wpn1 = wpn1 + weight;
				}
				node_map[instance].push_back(0); //pushing back fix;
				part_count++;
			}
			node_file.close();

			wptot = wp1 + wpn1;

			//writing into files
			if (part_count == 3336)
			{
				output_f.open("Sain_Kushwaha_output_FPGA-example1_rc1.txt");
				output_f << "Benchmark Name : FPGA-example1" << endl;
				is_benc = true;
			}
			else if (part_count == 542239)
			{
				output_f.open("Sain_Kushwaha_output_FPGA-example2_rc1.txt");
				output_f << "Benchmark Name : FPGA-example2" << endl;
				is_benc = true;
			}
			else if (part_count == 427800)
			{
				output_f.open("Sain_Kushwaha_output_FPGA-example3_rc1.txt");
				output_f << "Benchmark Name : FPGA-example3" << endl;
				is_benc = true;
			}
			else if (part_count == 844184)
			{
				output_f.open("Sain_Kushwaha_output_FPGA-example4_rc1.txt");
				output_f << "Benchmark Name : FPGA-example4" << endl;
				is_benc = true;
			}

			net_file.open(net_f.c_str());
			while (getline(net_file, netline))
			{
				count1 = 0;
				countn1 = 0;
				if (netline.substr(0, 3) == "net")
				{
					int a = 0;
					while (netline.substr(a + 4, 1) != " ")
					{
						a = a++;
					}
					netname = netline.substr(4, a);   //NET is HERE.
					getline(net_file, net_nodeline);

					while (net_nodeline.substr(0, 6) != "endnet")
					{
						int b = 0;
						while (net_nodeline.substr(b + 1, 1) != " ")
						{
							b = b++;
						}
						sep_node = split(net_nodeline.substr(1, b), '_');  //NODE is HERE.
						istringstream(sep_node[1]) >> nodename;
						node_net_map[nodename].push_back(netname);
						net_node_map[netname].push_back(nodename);

						if (node_map[nodename][1] == 1)
						{
							count1 = count1 + 1;
						}
						else
						{
							countn1 = countn1 + 1;
						}

						getline(net_file, net_nodeline);
					}
					net_card[netname].push_back(count1);
					net_card[netname].push_back(countn1);
				}
			}
			net_file.close();

			//Removing Duplicates
			for (net_nodeiter iter = node_net_map.begin(); iter != node_net_map.end(); iter++)
			{
				node_net_map[iter->first].erase(unique(node_net_map[iter->first].begin(), node_net_map[iter->first].end()), node_net_map[iter->first].end());
			}

			//Randomizing the Initial Cutset
	wp1 = 0;
	wpn1 = 0;
	int pres_node = 0;
	for (nodeiter g = node_map.begin(); g != node_map.end(); g++)
	{
		pres_node = g->first;
		if ((wp1 / wptot) < 0.25)
		{
			node_map[pres_node][1] = 1;
			wp1 = wp1 + node_map[pres_node][0];
		}
		else if (((wp1 / wptot) > 0.25) && ((wpn1 / wptot) < 0.25))
		{
			node_map[pres_node][1] = -1;
			wpn1 = wpn1 + node_map[pres_node][0];
		}
		else if (((wp1 / wptot) < 0.50) && ((wpn1 / wptot) > 0.25))
		{
			node_map[pres_node][1] = 1;
			wp1 = wp1 + node_map[pres_node][0];
		}
		else
		{
			node_map[pres_node][1] = -1;
			wpn1 = wpn1 + node_map[pres_node][0];
		}
	}

	//erasing entire net card map
	net_card.clear();

	//recalculating net cardinality after removing duplicates
	for (netnodeiter t = net_node_map.begin(); t != net_node_map.end(); t++)
	{
		count1 = 0;
		countn1 = 0;
		newnetname = t->first;
		for (int y = 0; y < t->second.size(); y++)
		{
			if (node_map[net_node_map[newnetname][y]][1] == 1)
			{
				count1 = count1 + 1;
			}
			if (node_map[net_node_map[newnetname][y]][1] == -1)
			{
				countn1 = countn1 + 1;
			}
		}
		net_card[newnetname].push_back(count1);
		net_card[newnetname].push_back(countn1);
	}


			//Initial Gain Bucket Calculation
			for (int i = 0; i < free_nodes.size(); i++)
			{
				int current_node = free_nodes[i];
				gain = 0;
				for (int var = 0; var < node_net_map[current_node].size(); var++)
				{
					current_net = node_net_map[current_node].at(var);
					net_count1 = net_card[current_net].at(0);
					net_countn1 = net_card[current_net].at(1);
					current_part = node_map[current_node].at(1);
					if (current_part == 1)
					{
						if (net_count1 == 1)
						{
							gain = gain + 1;
						}
						else if (net_countn1 == 0)
						{
							gain = gain - 1;
						}
					}
					else
					{
						if (net_count1 == 0)
						{
							gain = gain - 1;
						}
						else if (net_countn1 == 1)
						{
							gain = gain + 1;
						}
					}
				}
				gain_bucket[gain].push_back(current_node);
				node_map[current_node].push_back(gain);
			}

			//Initial Cutset 
			for (netcarditer a = net_card.begin(); a != net_card.end(); a++)
			{
				if ((net_card[a->first][0] != 0) && (net_card[a->first][1] != 0))
				{
					cutset = cutset + 1;
					initial_cutset = cutset;
				}
			}

			for (fg = 1; fg < 9; fg++)
			{
				cutset = cutset_cal(cutset);
			}
		}
	}

	int stop_s = clock(); //Timing of code ends here

	int time_ms = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	int time_hours = (time_ms) / (1000 * 60 * 60);
	int time_minutes = (time_ms - time_hours * (60 * 60 * 1000)) / (1000 * 60);
	int time_seconds = (time_ms - time_minutes * (1000 * 60)) / (1000);
	cout << "Execution Time:" << endl;
	cout << "HH: " << time_hours << " MM: " << time_minutes << " SS: " << time_seconds << endl; //Printing out total execution time.
	cout << "Execution Time: " << time_ms << endl;

	if (is_benc)
	{
		output_f << "Execution Time: " << " HH: " << time_hours << " MM: " << time_minutes << " SS: " << time_seconds << endl;
		output_f << "Starting Cut: " << initial_cutset << endl;
		output_f << "Final Cut: " << cutset << endl;
		output_f << "Percentage Change: " << (((initial_cutset - cutset) / initial_cutset) * 100) << "%" << endl;
		output_f << "Ratio Cut: " << (wp1 / wpn1) << endl;
	}

	output_f.close();
	return 0;
}
