:- lib(ic).
:- lib(ic_edge_finder).
:- lib(branch_and_bound).% for minimize/2
 
zip([], [], []).
zip([X|Xs], [Y|Ys], [v(X,Y)|L]) :- zip(Xs, Ys, L).
 
obter_dados(Ids) :-
        findall(Id, tarefa(Id, _, _, _), Ids).
 
/*  ===  apt: comentado
my_member([(ID_, _)|L], ST, ID) :- ID \= ID_, my_member(L, ST, ID).
my_member([(ID, ST)|_], ST, ID).
*/


/*
    Estilo de programação: implementar pela afirmativa
    Erro:  Havia troca dos dois últimos argumentos do predicado
*/
my_member([v(ID,ST)|_], ID,ST) :- !.      % implementar pela "afirmativa"
my_member([_|L],ID,ST) :- my_member(L,ID,ST).
 
 
selecionar_sucessores([],_,[]).
selecionar_sucessores([Id|SL], ST_ID, [ST|Succeding_StartTimes]) :-
        my_member(ST_ID,Id,ST),
        %select(ST, Id, ST_ID),
        selecionar_sucessores(SL, ST_ID, Succeding_StartTimes).
 
aplicar_precedencia([], _).
aplicar_precedencia([ST_y|L], ET_x) :- ST_y #>= ET_x, 
        aplicar_precedencia(L,ET_x).
%  Erro:  Havia troca dos dois argumentos na chamada recursiva do predicado



restr_precedencia([],[], _, _).
restr_precedencia([Id|Ids],[ST_x|SL], ST_ID, Concl) :-
        tarefa(Id, Succ, Dur_x, _),
        ET_x #= ST_x + Dur_x,
        Concl #>= ET_x,
        selecionar_sucessores(Succ, ST_ID, Succeding_StartTimes),
        aplicar_precedencia(Succeding_StartTimes, ET_x),
        restr_precedencia(Ids, SL, ST_ID, Concl).
 
test :- solve('/Users/afonsobrandao/CLP/base_dados.ecl').

solve(Instance) :-
        %compile('/Users/afonsobrandao/CLP/base_dados.ecl'),
	    compile(Instance),
        obter_dados(Ids),
        length(Ids, TaskNo),
        length(StartTimes, TaskNo),
 
        StartTimes #:: 0..1.0Inf,
        Concl #::0..1.0Inf,
       
        zip(Ids, StartTimes, ST_ID),
       
        restr_precedencia(Ids, StartTimes, ST_ID, Concl),
        get_min(Concl, Concl),
        escrever_solucao(ST_ID),
        write(Concl).


escrever_solucao([]).
escrever_solucao([v(ID, ST)|T]):- write(ID:ST), nl, escrever_solucao(T).

get_min_st([]).
get_min_st([ST|SL]) :- get_min(ST, ST), get_min_st(SL).

sum_list(Xs, Sum) :- sum_list(Xs, 0, Sum).
sum_list([], Sum, Sum).
sum_list([X|Xs], Sum0, Sum) :-
    Sum1 is Sum0 + X,
    sum_list(Xs, Sum1, Sum).

computeEndTimes([], [], []).
computeEndTimes([Start_Time|SL], [Duration|DL], [End_Time|EL]) :- 
        End_Time #= Start_Time + Duration,
        computeEndTimes(SL, DL, EL).

solve2 :-
    compile('/Users/afonsobrandao/CLP/base_dados.ecl'),
    %compile(Instance),
    obter_dados(Ids),
    length(Ids, TaskNo),
    length(StartTimes, TaskNo),

    StartTimes #:: 0..1.0Inf,
    Concl #::0..1.0Inf,
   
    zip(Ids, StartTimes, ST_ID),
   
    restr_precedencia(Ids, StartTimes, ST_ID, Concl),

    findall(Res, tarefa(_, _, _, Res), Resources),
    findall(Dur, tarefa(_, _, Dur, _), Durations),

    sum_list(Resources, MaxSum),
    MinRes :: 1 .. MaxSum,
    cumulative(StartTimes, Durations, Resources, MinRes),
    get_min(Concl, Concl),
    get_min_st(StartTimes),
    term_variables([StartTimes, MinRes], Vars),
    minimize(labeling(Vars), MinRes),

    write(StartTimes).

%critical([], []).
%critical([v(St, _)|ST_ID], [St|CriticalTasks]) :-
%    get_domain_size(St,0).

%critical([v(St, _)|ST_ID], [C|CriticalTasks]) :-
%    critical(T, CriticalTasks),
%    (get_domain_size(St, Size),
%    Size =/ 0,
%     critical(ST_ID)

solve3 :- 
    compile('/Users/afonsobrandao/CLP/base_dados.ecl'),
    %compile(Instance),
    obter_dados(Ids),
    length(Ids, TaskNo),
    length(StartTimes, TaskNo),

    StartTimes #:: 0..1.0Inf,
    Concl #::0..1.0Inf,
   
    zip(Ids, StartTimes, ST_ID),
   
    restr_precedencia(Ids, StartTimes, ST_ID, Concl),

    findall(Res, tarefa(_, _, _, Res), Resources),
    findall(Dur, tarefa(_, _, Dur, _), Durations),

    sum_list(Resources, MaxSum),
    MinRes :: 1 .. MaxSum,
    cumulative(StartTimes, Durations, Resources, MinRes),
    term_variables([StartTimes, MinRes], Vars),
    get_min(Concl, Concl),
    minimize(labeling(Vars), MinRes),
    write(Concl),


    
    write(StartTimes).
    
