with num_titles (c) as (
    select count(1)
    from titles
)

select cast((t.premiered / 10) * 10 as text) || 's' as decade,
       round(cast(count(1) as real) * 100 / (select c from num_titles), 4) as percentage
from titles as t
where t.premiered is not null
group by t.premiered / 10 /* group by each decade*/
order by percentage desc;
