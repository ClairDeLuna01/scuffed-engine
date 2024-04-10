#include <vector>
#include <unordered_map>
#include <set>

#include "typedef.hpp"

// deterministic finite automaton transition
// from -> to on symbol
// for compile time checking
template <i32 From, i32 Symbol, i32 To>
struct DFATransition
{
    static constexpr i32 from = From;
    static constexpr i32 symbol = Symbol;
    static constexpr i32 to = To;
};

struct DFATransitionRuntime
{
    i32 from;
    i32 symbol;
    i32 to;
};

// Check if two transitions are equal
template <class T1, class T2>
constexpr bool isEqual()
{
    return T1::from == T2::from && T1::symbol == T2::symbol;
}

template <class T, class... Ts>
struct isUniqueTransition;

template <class T>
struct isUniqueTransition<T> : std::true_type
{
};

template <class T, class T2, class... Ts>
struct isUniqueTransition<T, T2, Ts...> : std::integral_constant<bool,
                                                                 !isEqual<T, T2>() && isUniqueTransition<T, Ts...>::value>
{
};

template <class... Ts>
struct DFACheck;

template <class T>
struct DFACheck<T> : std::true_type
{
};

template <class T, class... Ts>
struct DFACheck<T, Ts...> : std::integral_constant<bool, isUniqueTransition<T, Ts...>::value && DFACheck<Ts...>::value>
{
};

// deterministic finite automaton state
// ACCEPTING: state is accepting
// REJECTING: state is rejecting
// ERROR: state is error
enum DFAState
{
    ACCEPTING,
    REJECTING,
    ERROR
};

// read type
// READ_NORMAL: read normal, valid symbol
// READ_EPSILON: read epsilon, no symbol
// READ_INVALID: read invalid, invalid symbol
// it is important to distinguish between epsilon and valid symbols
// since epsilon transitions do not change the read symbol
enum ReadType
{
    // Invalid symbol
    READ_INVALID,

    // Normal symbol
    READ_NORMAL,

    // Epsilon symbol
    READ_EPSILON
};

#define e 0

// deterministic finite automaton
class DFA
{
protected:
    i32 currentState;
    std::vector<i32> acceptingStates;
    std::vector<DFATransitionRuntime> transitions;
    std::unordered_map<u64, DFATransitionRuntime> transitionMap;

    std::function<std::string(i32)> state_to_string = [](i32 state)
    { return std::to_string(state); };
    std::function<std::string(i32)> input_to_string = [](i32 input)
    {
        if (input == e)
        {
            return std::string("e");
        }
        return std::to_string(input);
    };

    bool printDebugInfo = false;

    // get the transition from a state on a symbol
    // since the DFA is deterministic, there is only one transition for each state and symbol
    // if there is no transition, return transition with from = -1
    DFATransitionRuntime getTransition(i32 from, i32 symbol)
    {
        // check if valid non-epsilon transition is present
        for (DFATransitionRuntime transition : transitions)
        {
            if (transition.from == from && transition.symbol == symbol)
            {
                return transition;
            }

            if (transition.from == from && transition.symbol == e)
            {
                return {transition.from, e, transition.to};
            }
        }

        return {-1, -1, -1};
    }

    std::set<i32> getStates()
    {
        std::set<i32> states;
        for (DFATransitionRuntime transition : transitions)
        {
            states.insert(transition.from);
            states.insert(transition.to);
        }

        return states;
    }

    std::set<i32> getSymbols()
    {
        std::set<i32> symbols;
        for (DFATransitionRuntime transition : transitions)
        {
            symbols.insert(transition.symbol);
        }

        return symbols;
    }

    void computeTransitionMap()
    {
        std::set<i32> states = getStates();
        std::set<i32> symbols = getSymbols();

        union
        {
            struct
            {
                i32 state;
                i32 symbol;
            };
            u64 key;
        } u;

        for (i32 state : states)
        {
            for (i32 symbol : symbols)
            {
                u.state = state;
                u.symbol = symbol;
                transitionMap[u.key] = getTransition(state, symbol);
            }
        }
    }

    bool isAccepting(i32 state)
    {
        for (i32 acceptingState : acceptingStates)
        {
            if (state == acceptingState)
            {
                return true;
            }
        }
        return false;
    }

public:
    template <typename... Transitions, std::enable_if_t<DFACheck<Transitions...>::value, bool> = true>
    DFA(i32 initialState,
        std::vector<i32> acceptingStates,
        std::function<std::string(i32)> state_to_string,
        std::function<std::string(i32)> input_to_string,
        Transitions... staticTransitions) : currentState(initialState),
                                            acceptingStates(acceptingStates),
                                            state_to_string(state_to_string),
                                            input_to_string(input_to_string)
    {
        // convert the compile time transitions to runtime transitions
        (transitions.push_back({Transitions::from, Transitions::symbol, Transitions::to}), ...);

        for (DFATransitionRuntime transition : transitions)
        {
            if (printDebugInfo)
            {
                std::cout << transition.from << " --(" << (transition.symbol == e ? std::string("e") : std::to_string(transition.symbol)) << ")--> " << transition.to << "\n";
            }
        }

        computeTransitionMap();
    }

    template <typename... Transitions, std::enable_if_t<DFACheck<Transitions...>::value, bool> = true>
    DFA(i32 initialState,
        std::vector<i32> acceptingStates,
        Transitions... staticTransitions) : currentState(initialState),
                                            acceptingStates(acceptingStates)
    {
        // convert the compile time transitions to runtime transitions
        (transitions.push_back({Transitions::from, Transitions::symbol, Transitions::to}), ...);

        for (DFATransitionRuntime transition : transitions)
        {
            if (printDebugInfo)
            {
                std::cout << transition.from << " --(" << (transition.symbol == e ? std::string("e") : std::to_string(transition.symbol)) << ")--> " << transition.to << "\n";
            }
        }

        computeTransitionMap();
    }

    ReadType step(i32 symbol)
    {
        union
        {
            struct
            {
                i32 state;
                i32 symbol;
            };
            u64 key;
        } u = {currentState, symbol};

        DFATransitionRuntime transition = transitionMap[u.key];
        if (printDebugInfo)
        {
            std::cout << transition.from << " --(" << (transition.symbol == e ? std::string("e") : std::to_string(transition.symbol)) << ")--> " << transition.to << "\n";
        }
        if (transition.to == -1)
        {
            return READ_INVALID;
        }

        currentState = transition.to;
        return transition.symbol == e ? READ_EPSILON : READ_NORMAL;
    }

    bool run(std::vector<i32> input)
    {
        // verify that the input is valid
        for (i32 symbol : input)
        {
            if (symbol == e)
            {
                return false;
            }
        }
        u32 i = 0;
        while (i < input.size())
        {
            if (printDebugInfo)
            {
                std::cout << "Current state: " << state_to_string(currentState) << "\n";
                std::cout << "Current input: " << input_to_string(input[i]) << "\n";
            }
            ReadType readType = step(input[i]);
            if (readType == READ_INVALID)
            {
                return false;
            }
            else if (readType == READ_NORMAL)
            {
                i++;
            }
        }

        return isAccepting(currentState);
    }

    void printTransitionMap()
    {
        for (auto const &[key, transition] : transitionMap)
        {
            union
            {
                struct
                {
                    i32 state;
                    i32 symbol;
                };
                u64 key;
            } u;
            u.key = key;

            std::cout << u.state << " --(" << (u.symbol == e ? std::string("e") : std::to_string(u.symbol)) << ")--> " << transition.to << "\n";
        }
    }
};