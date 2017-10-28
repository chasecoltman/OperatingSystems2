/*
* Elevator Look
*/
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data{
	struct list_head queue;
	int direction;
}

static void look_merged_requests (struct request_queue *q, struct request *rq, struct request *next)
{
    list_del_init(&next->queuelist);
}

static int look_dispatch(struct request_queue *q, int force)
{
	
}

static void look_add_request(struct request_queue *q, struct request *rq)
{
	
}

static struct *look_former_request(struct request_queue *q, struct request *rq)
{
	
}

static struct *look_latter_request(struct request_queue *q, struct request *rq)
{
	
}

static int look_init_queue(struct request_queue *q, struct elevator_type *e)
{
	
}

static void look_exit_queue(struct elevator_queue *e)
{
	
}

static struct elevator_type elevator_look = {
    .ops = {
        .elevator_merge_req_fn 		= look_merged_requests,
        .elevator_dispatch_fn 		= look_dispatch,
        .elevator_add_req_fn 		= look_add_request,
        .elevator_former_req_fn 	= look_former_request,
        .elevator_latter_req_fn 	= look_latter_request,
        .elevator_init_fn 		= look_init_queue,
        .elevator_exit_fn 		= look_exit_queue,
    },
    .elevator_name = "look",
    .elevator_owner = THIS_MODULE,
};

static int __init noop_init(void)
{
	return elv_register(&elevator_noop);
}

static void __exit noop_exit(void)
{
	elv_unregister(&elevator_noop);
}

module_init(noop_init);
module_exit(noop_exit);


MODULE_AUTHOR("Chase Coltman, Alec Zitzelberger");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LOOK IO scheduler");