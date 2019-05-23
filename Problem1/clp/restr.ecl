:- ensure_loaded(utils).

%%
% computeTheEndTimes(Ids, StartTimes, StartTimesById, Concl)
%
% Apply restrictions to a task's end time
% and propagate them to its sucessors' start time
%
computeTheEndTimes([], [], _, _).
computeTheEndTimes([Id|Ids], [St|StartTimes], StartTimesById, Concl) :-
	tarefa(Id, SuccList, Dur, _),
	Et #= St + Dur,
	Concl #>= Et,
	filterById(StartTimesById, SuccList, SuccStartTimes),
	applyPrecedence(SuccStartTimes, Et),
	computeTheEndTimes(Ids, StartTimes, StartTimesById, Concl).

%%
% applyPrecedence(StartTimes, EndTime)
% 
% Applies the restriction for each St in StartTimes %St must come after EndTime
%
%
applyPrecedence([], _).
applyPrecedence([St|Sts], Et) :-
    St #>= Et,
    applyPrecedence(Sts, Et).

%%
% get_min_list(L)
%
% Apply get_min to all elements of a list
%
get_min_list([]).
get_min_list([H|T]) :- 
    get_min(H, H), 
    get_min_st(T).