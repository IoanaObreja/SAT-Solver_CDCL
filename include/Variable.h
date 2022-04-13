#ifndef VARIABLE_H
#define VARIABLE_H


class Variable
{
    public:
        Variable();
        Variable(int, int, int);
        virtual ~Variable();
        int level;
        int antecedent;
        int value;
        void operator = (Variable v) {
                level = v.level;
                antecedent = v.antecedent;
                value = v.value;
        }
    protected:
    private:
};

#endif // VARIABLE_H
