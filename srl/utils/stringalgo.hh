#include <string>
#include <vector>


template<class T>
unsigned int levenshtein_distance(const T &s1, const T & s2, std::vector<int>& source_to_target, std::vector<int>& target_to_source) {
	const size_t len1 = s1.size(), len2 = s2.size();
	std::vector<unsigned int> col(len2+1), prevCol(len2+1);
	std::vector<std::pair<int, int> > btcol((len2+1)*(len1+1));

	for (unsigned int j = 0; j < prevCol.size(); j++){
		prevCol[j] = j;
		btcol[j * (len1+1) ].first = j-1;
		btcol[j * (len1+1) ].second = 0;
	}
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i+1;
		btcol[i].second = i - 1;
		btcol[i].first = 0;
		for (unsigned int j = 0; j < len2; j++){
			int del = 1 + col[j];
			int ins = 1 + prevCol[1 + j];
			int sub = prevCol[j] + (s1[i]==s2[j] ? 0 : 1);

			if(del < ins){
				if(del < sub){
					col[j+1] = del;
					btcol[(j+1)*(len1+1) + i+1].first = i+1; 
					btcol[(j+1)*(len1+1) + i+1].second = j; 
				}else{
					col[j+1] = sub;
					btcol[(j+1)*(len1+1) + i+1].first = i+1; 
					btcol[(j+1)*(len1+1) + i+1].second = j+1; 
				}
			}else{
				if(ins < sub){
					col[j+1] = ins;
					btcol[(j+1)*(len1+1) + i+1].first = i; 
					btcol[(j+1)*(len1+1) + i+1].second = j+1; 
				}else{
					col[j+1] = sub;
					btcol[(j+1)*(len1+1) + i+1].first = i+1; 
					btcol[(j+1)*(len1+1) + i+1].second = j+1; 
				}
			}
		}
		col.swap(prevCol);
	}

	source_to_target.clear();
	source_to_target.resize(len1,-1);
	target_to_source.clear();
	target_to_source.resize(len2,-1);

	for(std::pair<int,int>* it = &(btcol[(len2+1)*(len1+1)-1]); it->first-1 >=0 && it->second-1 >=0; ){
		int source = it->first-1, target = it->second-1;
		if(source_to_target[source]==-1)
			source_to_target[source] = target;
		if(target_to_source[target]==-1)
			target_to_source[target] = source;
		it = &(btcol[target*(len1+1) + source]);
	}

	return prevCol[len2];
}