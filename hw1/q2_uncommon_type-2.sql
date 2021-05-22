with max_runtime_by_type (max_runtime, type)  as (
    select max(t.runtime_minutes), t.type
    from titles as t
    group by t.type
)

select t.type, t.primary_title, t.runtime_minutes
from titles as t
inner join max_runtime_by_type as m
on t.runtime_minutes = m.max_runtime and t.type = m.type
order by t.type asc, t.primary_title asc;
