#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>
#include <unordered_map>
#include <vector>
#include <set>

#define N_CHAR_IN_WORD (100000)

#define NAME_FILE "problem"
#define NUMBER_PROBLEM "3"

constexpr char name_input_file[] = NAME_FILE NUMBER_PROBLEM ".in";
constexpr char name_output_file[] = NAME_FILE NUMBER_PROBLEM ".out";

using namespace std;

enum class Errors: int
{
    SUCCESS,
    INPUT,
    OUTPUT,
    ALLOCATE,
    REALLOCATE,
    STR,
    STR_FULL
};

int error_handler(enum Errors e, ...)
{
    va_list args;
    va_start(args, e);
    switch (e) {
        case Errors::SUCCESS: break;
        case Errors::INPUT: vprintf("Error, unable to open input file \"%s\"\n", args); break;
        case Errors::OUTPUT: vprintf("Error, unable to create output file \"%s\"\n", args); break;
        case Errors::ALLOCATE: vprintf("Error, failed to allocate memory\n", args); break;
        case Errors::REALLOCATE: vprintf("Error, failed to reallocate memory\n", args); break;
        case Errors::STR: vprintf("Error, char in word not correct \"%c\"\n", args); break;
        case Errors::STR_FULL: vprintf("Error, word is too long\n", args); break;
    }
    va_end(args);

    return (int)e;
}

nullptr_t error_handler(enum Errors e, int *error, ...)
{
    *error = error_handler(e);
    return nullptr;
}

#define N_TYPE unsigned long
#define N_TYPE_SP "ld"
typedef N_TYPE n_type;
constexpr n_type MOD = 1e9 + 7;
//#define MOD ((n_type)(1e9 + 7))

class DFA
{
private:
    static constexpr char start_symbol = 'a';
    static constexpr char end_symbol = 'z';
    static constexpr char Alphabet_size = (end_symbol + 1) - start_symbol;
    struct State
    {
        bool isAccept = false;
        size_t transitions_next[Alphabet_size]{0};
        unordered_map<size_t, size_t> next_map;
        size_t transitions_prev[Alphabet_size]{0};
        unordered_map<size_t, size_t> prev_map;
    };
    State *states;
    set<size_t> AcceptStates;
    size_t n;
    size_t m;

public:
    explicit DFA(size_t n, size_t m) : n(n), m(m), states(new (nothrow) State[n + 1]) {}
    Errors check () {return (states) ? Errors::SUCCESS : Errors::ALLOCATE;}
    ~DFA() {delete[] states;}

    void change_state_accept(size_t st, bool isAccept) {states[st].isAccept = isAccept;}
    void add_accept_state(size_t st) {states[st].isAccept = true; AcceptStates.insert(st);}
    void add_transition(size_t a, size_t b, char s) {
        states[a].transitions_next[s - start_symbol] = b;
        states[a].next_map[b]++;
        states[b].transitions_prev[s - start_symbol] = a;
        states[b].prev_map[a]++;
    }
    bool check_word(const char *word) const
    {
        size_t cur_state = 1;
        for(size_t i = 0; word[i] != '\0'; i++)
        {
            cur_state = states[cur_state].transitions_next[word[i] - start_symbol];
            if(cur_state == 0) return false;
        }

        return states[cur_state].isAccept;
    }

    bool dfs_was_here(size_t cur, bool *was_here) const
    {
        if(was_here[cur]) return;
        was_here[cur] = true;

        for (int i = 0; i < Alphabet_size; ++i) {
            size_t new_cur = states[cur].transitions_prev[i];
            if(new_cur != 0) dfs_was_here(new_cur, was_here);
        }
    }

    typedef char byte;
    bool dfs_isCircle(size_t cur, byte *color, bool *useful_states) const
    {
        if(!useful_states[cur]) return false;
        if      (color[cur] == 'g') {printf("-1 %zu\n", cur); return true;}
        else if (color[cur] == 'b') {return false;}
        color[cur] = 'g';

        //for (int i = 0; i < Alphabet_size; ++i) {

        for(auto new_cur : states[cur].next_map) {

            //size_t new_cur = states[cur].transitions_next[i];
            if (dfs_isCircle(new_cur.first, color, useful_states)) return true;
        }

        color[cur] = 'b';
        return false;
    }

    n_type paths(size_t cur, n_type *n_paths) const
    {
        n_type result = 0;

        for (int i = 0; i < Alphabet_size; ++i) {
            size_t new_cur = states[cur].transitions_prev[i];
            if(new_cur != 0)
            {
                if(n_paths[new_cur] == -1) result += paths(new_cur, n_paths);
                else result += n_paths[new_cur];
                result %= MOD;
            }
        }

        return result;
    }

    n_type n_words() const
    {
        unique_ptr<bool> useful_states(new bool [n+1]);
        for(size_t i = 0; i < n+1; i++)
            useful_states.get()[i] = false;

        for(auto cur_state : AcceptStates)
            dfs_was_here(cur_state, useful_states.get());

        unique_ptr<byte> color(new byte [n+1]);
        for(size_t i = 0; i < n+1; i++)
            color.get()[i] = 'w';

        if(dfs_isCircle(1, color.get(), useful_states.get())) return -1;

        unique_ptr<n_type> n_paths(new n_type [n+1]);
        n_paths.get()[1] = 1;
        for(size_t i=2; i < n+1; i++)
            n_paths.get()[i] = -1;

        n_type result = states[1].isAccept;
        //AcceptStates.
        for(auto cur_state : AcceptStates)
        {
            if(!useful_states.get()[cur_state]) continue;
            result += paths(cur_state, n_paths.get());
            //printf("result - %" N_TYPE_SP "\n", result);
            result %= MOD;
        }

        return result;
    }

    n_type n_words_long() const
    {

        n_type result = states[1].isAccept;
        auto *cur_states = new unordered_map<size_t, size_t> {{1, 1}};

        size_t l_words = 2*min(n, m);
        size_t len = 0;

        while(!cur_states->empty() && (len++ < l_words))
        {
            auto *new_cur_states = new unordered_map<size_t, size_t>;

            for(auto cur_state : *cur_states) {
                for(auto new_state : states[cur_state.first].next_map)
                {
                    (*new_cur_states)[new_state.first] += (cur_state.second*new_state.second) % MOD;
                    (*new_cur_states)[new_state.first] %= MOD;
                    if(states[new_state.first].isAccept)
                    {
                        if(len > min(n, m))
                        {delete new_cur_states; delete cur_states; return -1;}
                    }
                }
//                for (int i = 0; i < Alphabet_size; ++i) {
//                    size_t new_state = states[cur_state.first].transitions_next[i];
//
//                    if(new_state != 0) {
//                        (*new_cur_states)[new_state] = ((*new_cur_states)[new_state] + cur_state.second)%MOD;
//
//                        if(states[new_state].isAccept)
//                        {
//                            if(len > min(n, m))
//                            {delete new_cur_states; delete cur_states; return -1;}
//                        }
//                    }
//                }
            }

            for(auto new_cur_state : *new_cur_states) {
                if(states[new_cur_state.first].isAccept)
                {
                    result = (result + new_cur_state.second)%MOD;
                }
            }

            delete cur_states;
            cur_states = new_cur_states;
        }

        delete cur_states;
        return result;
    }

    n_type n_words_with_len(size_t l) const
    {
        if(l == 0) return states[1].isAccept;

        n_type result = 0;
        auto *cur_states = new unordered_map<size_t, size_t>{{1, 1}};

        size_t l_words = l;

        while(!cur_states->empty() && l_words--)
        {
            auto *new_cur_states = new unordered_map<size_t, size_t>;

            for(auto cur_state : *cur_states) {
                for (int i = 0; i < Alphabet_size; i++) {
                    size_t new_state = states[cur_state.first].transitions_next[i];

                    if(new_state != 0) {
                        (*new_cur_states)[new_state] = ((*new_cur_states)[new_state]%MOD + cur_state.second%MOD)%MOD;
                    }
                }
            }

            delete cur_states;
            cur_states = new_cur_states;
        }

        for(auto cur_state : *cur_states) {
            if(states[cur_state.first].isAccept)
            {
                result = (result%MOD + cur_state.second%MOD)%MOD;
            }
        }

        delete cur_states;
        return result%MOD;
    }
};

char* scan_str(FILE *in, int *error)
{
    char *data = (char*)malloc((N_CHAR_IN_WORD + 1) * sizeof(char));
    if(!data) return error_handler(Errors::ALLOCATE, error);

    char buffer;
    size_t n;
    for(n = 0; fscanf(in, "%c", &buffer) != EOF && n < N_CHAR_IN_WORD; n++)
    {
        if(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t')
            break;
        if(buffer < 'a' || buffer > 'z')
        {
            free(data);
            *error = error_handler(Errors::STR, buffer);
            return nullptr;
        }
        data[n] = buffer;
    }
    if(n >= N_CHAR_IN_WORD && !(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t'))
    {
        free(data);
        *error = error_handler(Errors::STR_FULL, buffer);
        return nullptr;
    }
    data[n++] = '\0';

    //printf("%zu \"%s\"\n", n, data);

    char *new_ptr = (char*) realloc(data, n * sizeof (char));
    if(!new_ptr)
    {
        free(data);
        *error = error_handler(Errors::REALLOCATE);
        return nullptr;
    }
    return new_ptr;
}

DFA* scanDFA(FILE *in, int *error)
{
    size_t n, m, k;
    fscanf(in, "%zu %zu %zu", &n, &m, &k);
    DFA *dfa = new (nothrow) DFA(n, m);
    if(!dfa) return error_handler(Errors::ALLOCATE, error);
    if(error_handler(dfa->check()) != (int)Errors::SUCCESS)
    {
        delete dfa;
        return nullptr;
    }

    if(!k)
    {
        delete dfa;
        *error = -1;
        return nullptr;
    }
    while(k--)
    {
        size_t n_accept_state;
        fscanf(in, "%zu", &n_accept_state);
        dfa->add_accept_state(n_accept_state);
    }

    while(m--)
    {
        size_t a, b;
        char symbol;
        fscanf(in, "%zu %zu %c\n", &a, &b, &symbol);
        dfa->add_transition(a, b, symbol);
    }

    return dfa;
}

int print_answer(const char *result_str)
{
    unique_ptr<FILE, int(*)(FILE*)> out(fopen(name_output_file, "w"), fclose);
    if(!out) return error_handler(Errors::OUTPUT, name_output_file);

    fprintf(out.get(), "%s", result_str);

    return (int)Errors::SUCCESS;
}

int print_answer(const n_type result)
{
    unique_ptr<FILE, int(*)(FILE*)> out(fopen(name_output_file, "w"), fclose);
    if(!out) return error_handler(Errors::OUTPUT, name_output_file);

    fprintf(out.get(), "%" N_TYPE_SP, result);

    return (int)Errors::SUCCESS;
}

int main() {

    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    //unique_ptr<char, void (*)(void *)> str(scan_str(in.get(), &result), free);
    //if (!str) return result;

    unique_ptr<DFA> dfa(scanDFA(in.get(), &result));
    if (!dfa)
    {
        if (result == -1)
            return print_answer((n_type)0);

        return result;
    }



//    if(dfa->n_words())
//        return print_answer("Accepts");

    return print_answer(dfa->n_words());
}


/* 
 * quick start
cd 2sem_DM_1\C\build
..\..\run_cmake\run

 * set 
mkdir build & cd build & cmake -G "MinGW Makefiles" ..

* build and run
cmake --build . && (echo START & C)
echo return code: %errorlevel%

* or
..\..\run_cmake\run

* move
cd 2sem_DM_1\C\build
cd 2sem_DM_1\C
cd C\build

* update run_cmake
git submodule update --remote
*/