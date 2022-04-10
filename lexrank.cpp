/**
 * @file lexrank.cpp
 * @author Nari
 * @brief Continuous LexRank
 * @date 2022-03-28
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <string>
#include <cfloat>

using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::find;
using std::ifstream;
using std::string;
using std::vector;

/**
 * @brief 区切りたい文字(例 :)で区切られた文字列を返す
 *
 * @param str 文字列
 * @param sep 区切りたい文字
 * @return vector<string> 区切られた文字列
 */
vector<string> split(string str, string sep)
{
    if (sep == "")
        return {str};
    vector<string> result;
    string tstr = str + sep;
    long l = tstr.length(), sl = sep.length();
    string::size_type pos = 0, prev = 0;

    for (; pos < l && (pos = tstr.find(sep, pos)) != string::npos; prev = (pos += sl))
    {
        result.emplace_back(tstr, prev, pos - prev);
    }
    return result;
}

/**
 * @class CSR
 * @brief 疎行列をCSR形式で持つクラス
 */
class CSR
{
private:
    vector<double> values; // 疎行列のvalues
    vector<int> row_ptr;   // 疎行列のrow_ptr
    vector<int> col_idx;   // 疎行列のcol_idx

public:
    CSR(vector<double> values, vector<int> row_ptr, vector<int> col_idx)
    {
        this->values = values;
        this->row_ptr = row_ptr;
        this->col_idx = col_idx;
    };

    // 正規化を行う
    void normalization();
    // D^-1を求める
    CSR inv_diag();
    // 疎行列とベクトルの積を求める
    vector<double> prod(vector<double> vec);
    // 転置した疎行列とベクトルの積を求める
    vector<double> prod_t(vector<double> vec);
    // 疎行列の行数を取得する
    int get_nfile();
};

/**
 * @brief 正規化を行う
 */
void CSR::normalization()
{
    vector<double> sum(this->row_ptr.size() - 1, 0);

    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            sum[i] = sum[i] + (this->values[j] * this->values[j]);
        }
    }

    for (int i = 0; i < sum.size(); i++)
    {
        sum[i] = sqrt(sum[i]);
    }
    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            this->values[j] = this->values[j] / sum[i];
        }
    }
};
/**
 * @brief D^-1を求める
 * @return CSR D^-1
 */
CSR CSR::inv_diag()
{
    vector<double> d_values;
    vector<int> d_row_ptr;
    vector<int> d_col_idx;
    vector<double> e_vec(this->row_ptr.size() - 1, 1); // 全ての要素が1のベクトル
    vector<double> temp(this->row_ptr.size() - 1, 0);  // 一時的な計算結果を格納する
    vector<double> temp2(this->row_ptr.size() - 1, 0); // 一時的な計算結果を格納する

    // ここからS*S^T*eの計算
    // まずS^T*e
    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            //転置行列であることを考慮する
            temp[this->col_idx[j]] = temp[this->col_idx[j]] + this->values[j] * e_vec[i];
        }
    }
    // 次にS*temp
    // temp=(S^T*e)
    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            temp2[i] = temp2[i] + this->values[j] * temp[this->col_idx[j]];
        }
    }

    //ここでtemp2(S*S^T*e)をD^-1にする
    // temp2をd_values, d_row_ptr, d_col_idxにする
    copy(temp2.begin(), temp2.end(), back_inserter(d_values));
    // ここでd_valuesには0がある可能性があるが，d_row_ptrとd_col_idxを作成してから0を削除する

    int row_ptr_count = 0; // 元の行列のある行のd_valuesが全て0であるときrow_ptrにいれる値を適切なものにするため使用する

    for (int i = 0; i < d_values.size(); i++)
    {
        // d_values[i]が0でない場合
        if (d_values[i] != 0)
        {
            d_row_ptr.push_back(row_ptr_count);
            row_ptr_count++;
            d_col_idx.push_back(i);
        }
        else
        {
            d_row_ptr.push_back(row_ptr_count);
        }
    }
    d_row_ptr.push_back(row_ptr_count);

    // d_valuesの中で0がある場合削除する

    // for文内でd_valuesの中に０があるか探し，0がある場合trueになる
    bool flag = true;
    while (flag == true)
    {
        flag = false;
        for (int i = 0; i < d_values.size(); i++)
        {
            if (fabs(d_values[i]) < DBL_EPSILON)
            {
                flag = true;
                d_values.erase(d_values.begin() + i);
                break;
            }
        }
    };
    // DをD^-1にする
    for (int i = 0; i < d_values.size(); i++)
    {
        d_values[i] = 1 / d_values[i];
    }
    CSR d_csr(d_values, d_row_ptr, d_col_idx);
    return d_csr;
};
/**
 * @brief 疎行列とベクトルの積を求める
 * @param vec ベクトル
 * @return vector<double> 計算結果
 */
vector<double> CSR::prod(vector<double> vec)
{
    vector<double> temp(this->row_ptr.size() - 1, 0); //一時的な計算結果を格納する
    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            temp[i] = temp[i] + this->values[j] * vec[col_idx[j]];
        }
    }
    return temp;
};
/**
 * @brief 転置した疎行列とベクトルの積を求める
 * @param vec ベクトル
 * @return vector<double> 計算結果
 */
vector<double> CSR::prod_t(vector<double> vec)
{
    vector<double> temp(this->row_ptr.size() - 1, 0); //一時的な計算結果を格納する
    for (int i = 0; i < this->row_ptr.size() - 1; i++)
    {
        for (int j = this->row_ptr[i]; j < this->row_ptr[i + 1]; j++)
        {
            //転置行列を考慮する
            temp[this->col_idx[j]] = temp[this->col_idx[j]] + this->values[j] * vec[i];
        }
    }
    return temp;
};

/**
 * @brief 疎行列の行数を取得する
 * @return int 疎行列の行数
 */
int CSR::get_nfile()
{
    return this->row_ptr.size() - 1;
};

/**
 * @class LexRank
 * @brief Continuous LexRankを実装したクラス
 */
class LexRank
{
private:
    string filename;      //引数csv_fileを代入する
    double th;            //引数thresholdを代入する
    vector<int> file_ids; // file_idを格納する

public:
    LexRank(const char *csv_file, double threshold)
    {
        this->filename = csv_file;
        this->th = threshold;
    };
    // ファイルを読み込み,疎行列を持つCSRクラスのインスタンスを返す
    CSR csr_matrix();
    // べき乗法を行う
    int prop(int n, double d, vector<double> &u);
    // file_idsを取得する
    vector<int> get_file_ids();
};

/**
 * @brief　ファイルを読み込み,疎行列を持つCSRクラスのインスタンスを返す
 */
CSR LexRank::csr_matrix()
{
    vector<double> values;
    vector<int> row_ptr;
    vector<int> col_idx;
    string line, line2; // 1行ごとに読み込む際に使用する
    int e_count = 0;    //要素数をカウントする

    // ファイルの読み込み
    ifstream input_file(this->filename);
    if (!input_file.is_open())
    {
        cerr << "ファイルを開くことができません'" << this->filename << "'" << endl;
        std::exit(EXIT_SUCCESS);
    }
    // CSR形式として疎行列を取り扱う
    while (getline(input_file, line))
    {
        // ","があるか
        int pos = line.find(",");
        // posの結果で処理を変える
        // posがnposでelement_idとvalueがない(file_idのみ)場合
        if (pos == string::npos)
        {
            int file_id;
            // stoiを使用する際のエラー処理
            try
            {
                file_id = std::stoi(line);
                this->file_ids.push_back(file_id);
            }
            catch (const std::invalid_argument &e)
            {
                cout << "line invalid argument" << endl;
            }
            catch (const std::out_of_range &e)
            {
                cout << "Out of range" << endl;
            }
            // row_ptrにこの行のはじめの要素の番号を入れる
            row_ptr.push_back(e_count);
            // element_idとvalueがある場合
        }
        else
        {
            int file_id;
            // stoiを使用する際のエラー処理
            try
            {
                file_id = std::stoi(line.substr(0, pos));
                this->file_ids.push_back(file_id);
            }
            catch (const std::invalid_argument &e)
            {
                cout << "line invalid argument" << endl;
            }
            catch (const std::out_of_range &e)
            {
                cout << "Out of range" << endl;
            }
            // row_ptr
            row_ptr.push_back(e_count);
            // file_idを除いたe_id:valueを取り出す
            line2 = line.substr(pos + 1);
            for (const auto element : split(line2, ","))
            {
                // resultにcol_idxとvalueを入れていく
                vector<string> result = split(element, ":");
                // stoi,stodを使用する際のエラー処理
                try
                {
                    //負の値のカットする
                    if (std::stod(result[1]) < 0)
                    {
                        continue;
                    }
                    col_idx.push_back(std::stoi(result[0]));
                    values.push_back(std::stod(result[1]));
                }
                catch (const std::invalid_argument &e)
                {
                    cout << "result[0] or result[1] invalid argument" << endl;
                    break;
                }
                catch (const std::out_of_range &e)
                {
                    cout << "Out of range" << endl;
                }
                e_count++;
            }
        }
    }
    // 最後に全要素数がカウントされているe_countをいれる
    row_ptr.push_back(e_count);
    return CSR(values, row_ptr, col_idx);
};

/**
 * @brief べき乗法を行う
 * @param n べき乗法の反復回数
 * @param d テレポーテーションの確率
 * @param u 求める固有ベクトル
 * @return int 0
 */
int LexRank::prop(int n, double d, vector<double> &u)
{
    CSR csr = csr_matrix();
    csr.normalization();
    CSR d_csr = csr.inv_diag();
    int nfile = csr.get_nfile();

    double cons = d / nfile;              // d/N
    vector<double> p_vec(nfile, cons);    // P_n-1 //p_vec = cons_vec
    vector<double> e_vec(nfile, 1);       // 全ての要素が0のベクトル
    vector<double> cons_vec(nfile, cons); // 全ての要素がd/Nのベクトル

    for (int count = 0; count < n; count++)
    { // n:べき乗法の反復回数
        double norm = 0.0;
        vector<double> next_p_vec;     // P_n
        vector<double> temp(nfile, 0); // 計算結果を格納する

        // (S*S^T*D^-1*u_n-1)
        next_p_vec = csr.prod(csr.prod_t(d_csr.prod(p_vec)));

        // (1-d) * (S*S^T*D^-1*u_n-1)
        for (int i = 0; i < next_p_vec.size(); i++)
        {
            next_p_vec[i] = next_p_vec[i] * (1 - d);
        }

        // cons_vec + next_p_vec
        for (int i = 0; i < next_p_vec.size(); i++)
        {
            next_p_vec[i] = cons_vec[i] + next_p_vec[i];
        }

        // P_n - P_n-1
        for (int i = 0; i < next_p_vec.size(); i++)
        {
            temp[i] = next_p_vec[i] - p_vec[i];
        }

        //ノルムの計算
        for (int i = 0; i < temp.size(); i++)
        {
            norm += temp[i] * temp[i];
        }
        norm = sqrt(norm);

        // P_n-1にP_nを代入する
        p_vec = next_p_vec;
    };
    u = p_vec;
    return 0;
};

/**
 * @brief file_idsを返す
 * @return vector<int>
 */
vector<int> LexRank::get_file_ids()
{
    return this->file_ids;
};

int main(int argc, char *argv[])
{
    char *csv_file = argv[1]; // csv_file name
    int n;                    // べき乗法
    double d;                 // 減衰係数
    vector<double> u;         // prop関数に渡すときに使う
    vector<int> file_ids;     // file_idsを受け取る

    try
    {
        n = std::stoi(argv[2]);
    }
    catch (const std::invalid_argument &e)
    {
        cout << argv[2] << " : invalid argument" << endl;
    }
    try
    {
        d = std::stod(argv[3]);
    }
    catch (const std::invalid_argument &e)
    {
        cout << argv[3] << " : invalid argument" << endl;
    }

    LexRank lr(csv_file, d);
    lr.prop(n, d, u);

    std::ofstream ofs("./output.txt");
    file_ids = lr.get_file_ids();

    for (int i = 0; i < u.size(); i++)
    {
        ofs << file_ids[i] << ":" << u[i] << std::endl;
    }
    return 0;
}