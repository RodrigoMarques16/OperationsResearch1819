% Métodos de Apoio à Decisão
%   FCUP - SPRING 2019
%
% Task Planner
%
% [desc]
%
% Luís Brandão 201504440
%
% Rodrigo Marques 201605427
%

:- lib(ic).
:- lib(ic_global).
:- lib(ic_edge_finder).
:- lib(branch_and_bound).% for minimize/2
 
:- ensure_loaded(utils).
:- ensure_loaded(restr).

test :- solve(ex1).

%% solve(Instance)

%% 
% Main
%
solve(Instance) :-
	compile(Instance),
	getData(Ids, Durations, Workers),

    % Restrict lengths
	length(Ids, TaskNo),
	length(StartTimes, TaskNo),

    % Bounds 
	StartTimes #:: 0 .. 1.0Inf,
    Concl #:: 0 .. 1.0Inf,
    
    % Groups
    zip(Ids, StartTimes, StartTimesById),
    zip(Ids, Durations, DurationsById),
    zip(Ids, Workers, WorkersById),
    
    computeTheEndTimes(Ids, StartTimes, StartTimesById, Concl),

    % Minimum duration
    get_min(Concl, Concl),
    write('Project conclusion: '),
    writeln(Concl),

    sum_list(Workers, SumWorkers),
    MinWorkers #:: 1 .. SumWorkers,
    CritWorkers #:: 1 .. SumWorkers,

    % Critical tasks
    critical(StartTimesById, CritSts, CritDurs, CritWrks),
    writeln(CritSts),
    cumulative(CritSts, CritDurs, CritWrks, CritWorkers),
    get_min_list(CritSts),
    term_variables([CritSts, CritWorkers], V),
    labeling(V), 
    write('Critical workers: '),
    writeln(CritWorkers),

    % Minimum workers for fixed start time
    cumulative(StartTimes, Durations, Workers, MinWorkers),
    get_min_list(StartTimes),
    term_variables([StartTimes, MinWorkers], Vars),
    labeling(Vars),
    
    write('Fixed minimum workers: '),
    writeln(MinWorkers),

    minimumWorkers.
%%
% critical(Tasks, StartTimes, Durations, Workers)
%
% Instantiates StartTimes, Durations and Workers with values
% of critical tasks
%
critical([], [], [], []).
critical([v(Id,S)|Tasks], [S|Sts], [D|Ds], [W|Ws] ) :-
    get_domain_size(S,1),
    tarefa(Id, _, D, W),
    critical(Tasks, Sts, Ds, Ws), 
    !.
critical([_|Tasks], Sts, Ds, Ws) :-
    critical(Tasks, Sts, Ds, Ws).

%%
% minimumWorkers
%
% Prints the minimum number of workers for the loaded instance
%
minimumWorkers :-
    getData(Ids, Durations, Workers),

    % Restrict lengths
    length(Ids, TaskNo),
    length(StartTimes, TaskNo),

    % Bounds 
	StartTimes #:: 0 .. 1.0Inf,
    Concl #:: 0 .. 1.0Inf,
    
    % Groups
    zip(Ids, StartTimes, StartTimesById),
    zip(Ids, Durations, DurationsById),
    zip(Ids, Workers, WorkersById),
    
    computeTheEndTimes(Ids, StartTimes, StartTimesById, Concl),

    get_min(Concl, Concl),

    % Minimum workers 
    sum_list(Workers, SumWorkers),
    MinWorkers #:: 1 .. SumWorkers,

    cumulative(StartTimes, Durations, Workers, MinWorkers),
    term_variables([StartTimes, MinWorkers], Vars),
    minimize(labeling(Vars), MinWorkers),
    
    write('Minimum workers: '),
    writeln(MinWorkers).

printSolution([]).
printSolution([v(Id, St)|T]) :-
    writeln(Id:St),
    printSolution(T).
