.headers on

with movie_ratings (title_id, primary_title, rating, votes) as (
    select t.title_id, t.primary_title, r.rating, r.votes
    from titles as t
    inner join ratings as r on t.title_id = r.title_id
    where t.type = 'movie'
),
num_votes (num) as (
    select sum(votes) from movie_ratings
),
C (c) as (
    /* weighted average rating of all movies */
    select sum(mr.rating * mr.votes) / num_votes.num
    from movie_ratings as mr, num_votes
),
m (minimum) as (
    select 25000
),
weighted_ratings (primary_title, WR) as (
    select primary_title, (C * m.minimum) / cast((mr.votes + m.minimum) as real) +
                        (mr.rating * mr.votes) / cast((mr.votes + m.minimum) as real)
    from movie_ratings as mr, C, m
)


select * from weighted_ratings
order by WR desc
limit 250;
