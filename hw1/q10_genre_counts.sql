.headers on

with recursive cte (g, org_str, idx) as (
    /* Initial state: ('' (the genre we extract), 'a,b,c,', 1 (the next starting idx)) */
    select '', t.genres || ',', 1
    from titles as t
    where (t.genres is not null) and (t.genres != '') and (t.genres != '\N')
    union all
    /* ('', 'a,b,c,', 1) --> ('a', 'a,b,c,', 3) this will be added to the queue */
    /* ('c', 'a,b,c,', 7) --> this will not generate new tuple because 7 > its length*/
    select substr(org_str, idx, instr(substr(org_str, idx), ',')-1),
           org_str,
           idx+instr(substr(org_str, idx), ',')
    from cte
    where idx <= length(org_str)
),
genres (genre) as (
    select distinct g
    from cte
    where g != ''
)

select g.genre, count(t.title_id) as cnt
from genres as g
inner join titles as t
on instr(t.genres, g.genre) != 0 /* if this title contains that genre */
group by g.genre /* for each genre */
order by cnt desc;


/* Actually, we can also just directly count the cte table */
/* select g as genre, count(1) as cnt */
/* from cte */
/* where g != '' */
/* group by genre */
/* order by cnt desc; */
